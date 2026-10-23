#include "pch.h"
#include "ConversationInstance.h"
#include "ConversationParticipantComponent.h"
#include "ConversationRegistry.h"
#include "ConversationTaskNode.h"
#include "Kismet/KismetTextLibrary.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UConversationInstance::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationInstance"));

    return Class;
}

bool UConversationInstance::HasServerLayout()
{
    auto Class = StaticClass();
    auto ParticipantsProperty = Class ? Class->FindPropertyByName(FName(TEXT("Participants"))) : nullptr;

    return ParticipantsProperty && Class->GetPropertiesSize() == (ParticipantsProperty->GetOffset_ForInternal() == sizeof(UObject) ? 0x190 : 0x1A8);
}

void UConversationInstance::ServerRemoveParticipant(FGameplayTag ParticipantID)
{
    for (int32 Index = 0; Index < GetParticipants().List.Num(); Index++)
    {
        if (!(GetParticipants().List[Index].ParticipantID == ParticipantID))
            continue;

        if (auto OldParticipant = GetParticipants().List[Index].GetParticipantComponent())
        {
            if (GetbConversationStarted())
                OldParticipant->ServerNotifyConversationEnded(this);
        }

        GetParticipants().List.RemoveAt(Index);
        break;
    }
}

void UConversationInstance::ServerAssignParticipant(FGameplayTag ParticipantID, AActor* ParticipantActor)
{
    if (!ParticipantID.IsValid() || !ParticipantActor)
        return;

    ServerRemoveParticipant(ParticipantID);

    FConversationParticipantEntry NewEntry;
    NewEntry.ParticipantID = ParticipantID;
    NewEntry.Actor = ParticipantActor;
    GetParticipants().List.Add(NewEntry);

    if (GetbConversationStarted())
    {
        if (auto ParticipantComponent = NewEntry.GetParticipantComponent())
            ParticipantComponent->ServerNotifyConversationStarted(this, ParticipantID);
    }
}

void UConversationInstance::ServerStartConversation(FGameplayTag EntryPoint)
{
    ResetConversationProgress();
    GetStartingEntryGameplayTag() = EntryPoint;

    auto ConversationRegistry = UConversationRegistry::GetFromWorld(GetWorld());

    TArray<FGuid> PotentialStartingPoints = ConversationRegistry ? ConversationRegistry->GetOutputLinkGUIDs(EntryPoint) : TArray<FGuid>();

    if (PotentialStartingPoints.Num() == 0)
    {
        ServerAbortConversation();
        return;
    }

    TArray<FGuid> LegalStartingPoints = DetermineBranches(PotentialStartingPoints, EConversationRequirementResult::FailedButVisible);

    if (LegalStartingPoints.Num() == 0)
    {
        ServerAbortConversation();
        return;
    }

    const int32 StartingIndex = GetConversationRNG().RandRange(0, LegalStartingPoints.Num() - 1);

    FConversationBranchPoint StartingPoint;
    StartingPoint.ClientChoice.ChoiceReference = FConversationChoiceReference(LegalStartingPoints[StartingIndex]);

    GetStartingBranchPoint() = StartingPoint;
    GetCurrentBranchPoint() = StartingPoint;

    const TArray<FConversationParticipantEntry> ParticipantList = GetParticipants().List;

    for (const FConversationParticipantEntry& ParticipantEntry : ParticipantList)
    {
        if (auto ParticipantComponent = ParticipantEntry.GetParticipantComponent())
            ParticipantComponent->ServerNotifyConversationStarted(this, ParticipantEntry.ParticipantID);
    }

    GetbConversationStarted() = true;

    TryStartingConversation();
}

void UConversationInstance::TryStartingConversation()
{
    if (!GetbConversationStarted())
        return;

    OnCurrentConversationNodeModified();
}

void UConversationInstance::ServerAdvanceConversation(const FAdvanceConversationRequest& InChoicePicked)
{
    if (!GetbConversationStarted() || !GetCurrentChoiceReference().IsValid())
        return;

    TArray<FConversationBranchPoint> CandidateDestinations;

    if (!(InChoicePicked.Choice == FConversationChoiceReference()))
    {
        const FConversationBranchPoint* BranchPoint = FindBranchPointFromClientChoice(InChoicePicked.Choice);

        if (!BranchPoint)
        {
            ServerAbortConversation();
            return;
        }

        CandidateDestinations.Add(*BranchPoint);
    }
    else
    {
        if (GetCurrentBranchPoints().Num() == 0 && GetScopeStack().Num() > 0)
        {
            const FConversationChoiceReference ScopeTop = GetScopeStack()[GetScopeStack().Num() - 1];
            ModifyCurrentConversationNode(ScopeTop);
            return;
        }

        for (const FConversationBranchPoint& BranchPoint : GetCurrentBranchPoints())
        {
            if (BranchPoint.ClientChoice.IsChoiceAvailable())
                CandidateDestinations.Add(BranchPoint);
        }
    }

    TArray<FConversationBranchPoint> ValidDestinations;
    FConversationContext Context = FConversationContext::CreateServerContext(this, nullptr);

    for (const FConversationBranchPoint& BranchPoint : CandidateDestinations)
    {
        auto TaskNode = Cast<UConversationTaskNode>(BranchPoint.ClientChoice.ChoiceReference.NodeReference.TryToResolve(Context));

        if (!TaskNode || TaskNode->CheckRequirements(Context) == EConversationRequirementResult::Passed)
            ValidDestinations.Add(BranchPoint);
    }

    if (ValidDestinations.Num() == 0)
    {
        ServerAbortConversation();
        return;
    }

    const int32 StartingIndex = GetConversationRNG().RandRange(0, ValidDestinations.Num() - 1);
    const FConversationBranchPoint TargetChoice = ValidDestinations[StartingIndex];

    ModifyCurrentConversationNode(TargetChoice);
}

void UConversationInstance::ServerAbortConversation()
{
    if (GetbConversationStarted())
    {
        const TArray<FConversationParticipantEntry> ListCopy = GetParticipants().List;

        for (const FConversationParticipantEntry& ParticipantEntry : ListCopy)
            ServerRemoveParticipant(ParticipantEntry.ParticipantID);
    }

    ResetConversationProgress();

    GetbConversationStarted() = false;
}

void UConversationInstance::PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& ClientMessage)
{
    FClientConversationMessagePayload LastMessage;
    LastMessage.Message = ClientMessage;
    LastMessage.Options = GetCurrentUserChoices();
    LastMessage.CurrentNode = Context.GetCurrentNodeHandle();
    LastMessage.Participants = GetParticipantsCopy();

    FCheckpoint Checkpoint;
    Checkpoint.ClientBranchPoint = GetCurrentBranchPoint();
    Checkpoint.ScopeStack = GetScopeStack();
    GetClientBranchPoints().Add(Checkpoint);

    for (const FConversationParticipantEntry& ParticipantEntry : LastMessage.Participants.List)
    {
        if (auto ParticipantComponent = ParticipantEntry.GetParticipantComponent())
            ParticipantComponent->SendClientConversationMessage(Context, LastMessage);
    }
}

void UConversationInstance::ReturnToLastClientChoice(const FConversationContext& Context)
{
    if (GetClientBranchPoints().Num() > 1)
    {
        GetClientBranchPoints().RemoveAt(GetClientBranchPoints().Num() - 1);

        const FCheckpoint Checkpoint = GetClientBranchPoints()[GetClientBranchPoints().Num() - 1];
        GetScopeStack() = Checkpoint.ScopeStack;
        ModifyCurrentConversationNode(Checkpoint.ClientBranchPoint);
    }
}

void UConversationInstance::ReturnToCurrentClientChoice(const FConversationContext& Context)
{
    if (GetClientBranchPoints().Num() > 0)
    {
        const FCheckpoint Checkpoint = GetClientBranchPoints()[GetClientBranchPoints().Num() - 1];

        GetClientBranchPoints().RemoveAt(GetClientBranchPoints().Num() - 1);

        GetScopeStack() = Checkpoint.ScopeStack;
        ModifyCurrentConversationNode(Checkpoint.ClientBranchPoint);
    }
}

void UConversationInstance::ReturnToStart(const FConversationContext& Context)
{
    const FGameplayTag EntryStartPointGameplayTagCache = GetStartingEntryGameplayTag();
    const FConversationBranchPoint StartingBranchPointCache = GetStartingBranchPoint();

    ResetConversationProgress();

    GetStartingEntryGameplayTag() = EntryStartPointGameplayTagCache;
    GetStartingBranchPoint() = StartingBranchPointCache;

    ModifyCurrentConversationNode(StartingBranchPointCache);
}

void UConversationInstance::ModifyCurrentConversationNode(const FConversationChoiceReference& NewChoice)
{
    FConversationBranchPoint BranchPoint;
    BranchPoint.ClientChoice.ChoiceReference = NewChoice;

    ModifyCurrentConversationNode(BranchPoint);
}

void UConversationInstance::ModifyCurrentConversationNode(const FConversationBranchPoint& NewBranchPoint)
{
    GetCurrentBranchPoint() = NewBranchPoint;

    for (const FConversationNodeHandle& ReturnScope : NewBranchPoint.ReturnScopeStack)
        GetScopeStack().Add(FConversationChoiceReference(ReturnScope));

    OnCurrentConversationNodeModified();
}

void UConversationInstance::ResetConversationProgress()
{
    GetStartingEntryGameplayTag() = FGameplayTag();
    GetStartingBranchPoint() = FConversationBranchPoint();
    GetCurrentBranchPoint() = FConversationBranchPoint();
    GetCurrentBranchPoints().Reset();
    GetClientBranchPoints().Reset();
    GetCurrentUserChoices().Reset();
}

void UConversationInstance::UpdateNextChoices(const FConversationContext& Context)
{
    TArray<FConversationBranchPoint> AllChoices;

    if (auto TaskNode = Cast<UConversationTaskNode>(GetCurrentChoiceReference().NodeReference.TryToResolve(Context)))
    {
        FConversationContext ChoiceContext = Context.CreateChildContext(TaskNode);

        TArray<FGuid> SourceGUIDs;
        SourceGUIDs.Add(GetCurrentChoiceReference().NodeReference.NodeGUID);

        const TArray<FGuid> CandidateDestinations = ChoiceContext.ConversationRegistry ? ChoiceContext.ConversationRegistry->GetOutputLinkGUIDs(SourceGUIDs) : TArray<FGuid>();

        FConversationBranchPointBuilder BranchBuilder;
        UConversationTaskNode::GenerateChoicesForDestinations(BranchBuilder, ChoiceContext, CandidateDestinations);
        AllChoices = BranchBuilder.BranchPoints;
    }

    SetNextChoices(AllChoices);
}

void UConversationInstance::SetNextChoices(const TArray<FConversationBranchPoint>& InAllChoices)
{
    GetCurrentUserChoices().Reset();
    GetCurrentBranchPoints() = InAllChoices;

    for (const FConversationBranchPoint& UserBranchPoint : GetCurrentBranchPoints())
    {
        if (UserBranchPoint.ClientChoice.ChoiceType != EConversationChoiceType::ServerOnly)
            GetCurrentUserChoices().Add(UserBranchPoint.ClientChoice);
    }

    if ((GetCurrentBranchPoints().Num() > 0 || GetScopeStack().Num() > 0) && GetCurrentUserChoices().Num() == 0)
    {
        FClientConversationOptionEntry DefaultChoice;
        DefaultChoice.ChoiceText = UKismetTextLibrary::Conv_StringToText(FString(TEXT("Continue")));
        DefaultChoice.ChoiceType = EConversationChoiceType::UserChoiceAvailable;
        GetCurrentUserChoices().Add(DefaultChoice);
    }
}

const FConversationBranchPoint* UConversationInstance::FindBranchPointFromClientChoice(const FConversationChoiceReference& InChoice) const
{
    for (const FConversationBranchPoint& BranchPoint : GetCurrentBranchPoints())
    {
        if (BranchPoint.ClientChoice.ChoiceType != EConversationChoiceType::ServerOnly && BranchPoint.ClientChoice == InChoice)
            return &BranchPoint;
    }

    return nullptr;
}

TArray<FGuid> UConversationInstance::DetermineBranches(const TArray<FGuid>& SourceList, EConversationRequirementResult MaximumRequirementResult)
{
    FConversationContext Context = FConversationContext::CreateServerContext(this, nullptr);

    TArray<FGuid> EnabledPaths;

    for (const FGuid& TestGUID : SourceList)
    {
        auto TaskNode = Context.ConversationRegistry ? Cast<UConversationTaskNode>(Context.ConversationRegistry->GetRuntimeNodeFromGUID(TestGUID)) : nullptr;

        if (TaskNode && (int64)TaskNode->CheckRequirements(Context) <= (int64)MaximumRequirementResult)
            EnabledPaths.Add(TestGUID);
    }

    return EnabledPaths;
}

void UConversationInstance::OnCurrentConversationNodeModified()
{
    FConversationContext AnonContext = FConversationContext::CreateServerContext(this, nullptr);
    auto TaskNode = Cast<UConversationTaskNode>(GetCurrentChoiceReference().NodeReference.TryToResolve(AnonContext));

    if (!TaskNode)
    {
        ServerAbortConversation();
        return;
    }

    FConversationContext Context = AnonContext.CreateChildContext(TaskNode);

    const FConversationTaskResult TaskResult = TaskNode->ExecuteTaskNodeWithSideEffects(Context);

    if (GetScopeStack().Num() > 0 && GetScopeStack()[GetScopeStack().Num() - 1].NodeReference.NodeGUID == TaskNode->GetNodeGuid())
        GetScopeStack().RemoveAt(GetScopeStack().Num() - 1);

    UpdateNextChoices(Context);

    switch (TaskResult.Type)
    {
    case EConversationTaskResultType::AbortConversation:
        ServerAbortConversation();
        break;
    case EConversationTaskResultType::AdvanceConversation:
        ServerAdvanceConversation(FAdvanceConversationRequest());
        break;
    case EConversationTaskResultType::AdvanceConversationWithChoice:
        ModifyCurrentConversationNode(TaskResult.AdvanceToChoice.Choice);
        break;
    case EConversationTaskResultType::PauseConversationAndSendClientChoices:
        PauseConversationAndSendClientChoices(Context, TaskResult.Message);
        break;
    case EConversationTaskResultType::ReturnToLastClientChoice:
        ReturnToLastClientChoice(Context);
        break;
    case EConversationTaskResultType::ReturnToCurrentClientChoice:
        ReturnToCurrentClientChoice(Context);
        break;
    case EConversationTaskResultType::ReturnToConversationStart:
        ReturnToStart(Context);
        break;
    default:
        break;
    }
}

uint8* UConversationInstance::GetLayoutMember(int32 OldLayoutOffset, int32 NewLayoutOffset) const
{
    static auto Participants = StaticClass()->FindPropertyByName(FName(TEXT("Participants")));

    return (uint8*)this + (Participants->GetOffset_ForInternal() == sizeof(UObject) ? OldLayoutOffset : NewLayoutOffset);
}

FConversationParticipants& UConversationInstance::GetParticipants() const
{
    static auto Participants = StaticClass()->FindPropertyByName(FName(TEXT("Participants")));

    return *Participants->ContainerPtrToValuePtr<FConversationParticipants>((void*)this);
}

FGameplayTag& UConversationInstance::GetStartingEntryGameplayTag() const
{
    return *(FGameplayTag*)GetLayoutMember(0x38, 0x60);
}

FConversationBranchPoint& UConversationInstance::GetStartingBranchPoint() const
{
    return *(FConversationBranchPoint*)GetLayoutMember(0x40, 0x68);
}

FConversationBranchPoint& UConversationInstance::GetCurrentBranchPoint() const
{
    return *(FConversationBranchPoint*)GetLayoutMember(0xC0, 0xE8);
}

TArray<UConversationInstance::FCheckpoint>& UConversationInstance::GetClientBranchPoints() const
{
    return *(TArray<FCheckpoint>*)GetLayoutMember(0x140, 0x168);
}

TArray<FConversationBranchPoint>& UConversationInstance::GetCurrentBranchPoints() const
{
    return *(TArray<FConversationBranchPoint>*)GetLayoutMember(0x150, 0x178);
}

TArray<FClientConversationOptionEntry>& UConversationInstance::GetCurrentUserChoices() const
{
    return *(TArray<FClientConversationOptionEntry>*)GetLayoutMember(0x160, 0x40);
}

TArray<FConversationChoiceReference>& UConversationInstance::GetScopeStack() const
{
    return *(TArray<FConversationChoiceReference>*)GetLayoutMember(0x170, 0x188);
}

FRandomStream& UConversationInstance::GetConversationRNG() const
{
    return *(FRandomStream*)GetLayoutMember(0x180, 0x198);
}

bool& UConversationInstance::GetbConversationStarted() const
{
    return *(bool*)GetLayoutMember(0x188, 0x1A0);
}
