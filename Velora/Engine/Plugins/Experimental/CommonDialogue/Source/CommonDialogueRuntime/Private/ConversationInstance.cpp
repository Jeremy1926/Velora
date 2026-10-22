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

    return ParticipantsProperty && ParticipantsProperty->GetOffset_ForInternal() == sizeof(UObject) && Class->GetPropertiesSize() == sizeof(UConversationInstance);
}

void UConversationInstance::ServerRemoveParticipant(FGameplayTag ParticipantID)
{
    for (int32 Index = 0; Index < Participants.List.Num(); Index++)
    {
        if (!(Participants.List[Index].ParticipantID == ParticipantID))
            continue;

        if (auto OldParticipant = Participants.List[Index].GetParticipantComponent())
        {
            if (bConversationStarted)
                OldParticipant->ServerNotifyConversationEnded(this);
        }

        Participants.List.RemoveAt(Index);
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
    Participants.List.Add(NewEntry);

    if (bConversationStarted)
    {
        if (auto ParticipantComponent = NewEntry.GetParticipantComponent())
            ParticipantComponent->ServerNotifyConversationStarted(this, ParticipantID);
    }
}

void UConversationInstance::ServerStartConversation(FGameplayTag EntryPoint)
{
    ResetConversationProgress();
    StartingEntryGameplayTag = EntryPoint;

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

    const int32 StartingIndex = ConversationRNG.RandRange(0, LegalStartingPoints.Num() - 1);

    FConversationBranchPoint StartingPoint;
    StartingPoint.ClientChoice.ChoiceReference = FConversationChoiceReference(LegalStartingPoints[StartingIndex]);

    StartingBranchPoint = StartingPoint;
    CurrentBranchPoint = StartingPoint;

    const TArray<FConversationParticipantEntry> ParticipantList = Participants.List;

    for (const FConversationParticipantEntry& ParticipantEntry : ParticipantList)
    {
        if (auto ParticipantComponent = ParticipantEntry.GetParticipantComponent())
            ParticipantComponent->ServerNotifyConversationStarted(this, ParticipantEntry.ParticipantID);
    }

    bConversationStarted = true;

    TryStartingConversation();
}

void UConversationInstance::TryStartingConversation()
{
    if (!bConversationStarted)
        return;

    OnCurrentConversationNodeModified();
}

void UConversationInstance::ServerAdvanceConversation(const FAdvanceConversationRequest& InChoicePicked)
{
    if (!bConversationStarted || !GetCurrentChoiceReference().IsValid())
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
        if (CurrentBranchPoints.Num() == 0 && ScopeStack.Num() > 0)
        {
            const FConversationChoiceReference ScopeTop = ScopeStack[ScopeStack.Num() - 1];
            ModifyCurrentConversationNode(ScopeTop);
            return;
        }

        for (const FConversationBranchPoint& BranchPoint : CurrentBranchPoints)
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

    const int32 StartingIndex = ConversationRNG.RandRange(0, ValidDestinations.Num() - 1);
    const FConversationBranchPoint TargetChoice = ValidDestinations[StartingIndex];

    ModifyCurrentConversationNode(TargetChoice);
}

void UConversationInstance::ServerAbortConversation()
{
    if (bConversationStarted)
    {
        const TArray<FConversationParticipantEntry> ListCopy = Participants.List;

        for (const FConversationParticipantEntry& ParticipantEntry : ListCopy)
            ServerRemoveParticipant(ParticipantEntry.ParticipantID);
    }

    ResetConversationProgress();

    bConversationStarted = false;
}

void UConversationInstance::PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& ClientMessage)
{
    FClientConversationMessagePayload LastMessage;
    LastMessage.Message = ClientMessage;
    LastMessage.Options = CurrentUserChoices;
    LastMessage.CurrentNode = Context.GetCurrentNodeHandle();
    LastMessage.Participants = GetParticipantsCopy();

    FCheckpoint Checkpoint;
    Checkpoint.ClientBranchPoint = CurrentBranchPoint;
    Checkpoint.ScopeStack = ScopeStack;
    ClientBranchPoints.Add(Checkpoint);

    for (const FConversationParticipantEntry& ParticipantEntry : LastMessage.Participants.List)
    {
        if (auto ParticipantComponent = ParticipantEntry.GetParticipantComponent())
            ParticipantComponent->SendClientConversationMessage(Context, LastMessage);
    }
}

void UConversationInstance::ReturnToLastClientChoice(const FConversationContext& Context)
{
    if (ClientBranchPoints.Num() > 1)
    {
        ClientBranchPoints.RemoveAt(ClientBranchPoints.Num() - 1);

        const FCheckpoint Checkpoint = ClientBranchPoints[ClientBranchPoints.Num() - 1];
        ScopeStack = Checkpoint.ScopeStack;
        ModifyCurrentConversationNode(Checkpoint.ClientBranchPoint);
    }
}

void UConversationInstance::ReturnToCurrentClientChoice(const FConversationContext& Context)
{
    if (ClientBranchPoints.Num() > 0)
    {
        const FCheckpoint Checkpoint = ClientBranchPoints[ClientBranchPoints.Num() - 1];

        ClientBranchPoints.RemoveAt(ClientBranchPoints.Num() - 1);

        ScopeStack = Checkpoint.ScopeStack;
        ModifyCurrentConversationNode(Checkpoint.ClientBranchPoint);
    }
}

void UConversationInstance::ReturnToStart(const FConversationContext& Context)
{
    const FGameplayTag EntryStartPointGameplayTagCache = StartingEntryGameplayTag;
    const FConversationBranchPoint StartingBranchPointCache = StartingBranchPoint;

    ResetConversationProgress();

    StartingEntryGameplayTag = EntryStartPointGameplayTagCache;
    StartingBranchPoint = StartingBranchPointCache;

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
    CurrentBranchPoint = NewBranchPoint;

    for (const FConversationNodeHandle& ReturnScope : NewBranchPoint.ReturnScopeStack)
        ScopeStack.Add(FConversationChoiceReference(ReturnScope));

    OnCurrentConversationNodeModified();
}

void UConversationInstance::ResetConversationProgress()
{
    StartingEntryGameplayTag = FGameplayTag();
    StartingBranchPoint = FConversationBranchPoint();
    CurrentBranchPoint = FConversationBranchPoint();
    CurrentBranchPoints.Reset();
    ClientBranchPoints.Reset();
    CurrentUserChoices.Reset();
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
    CurrentUserChoices.Reset();
    CurrentBranchPoints = InAllChoices;

    for (const FConversationBranchPoint& UserBranchPoint : CurrentBranchPoints)
    {
        if (UserBranchPoint.ClientChoice.ChoiceType != EConversationChoiceType::ServerOnly)
            CurrentUserChoices.Add(UserBranchPoint.ClientChoice);
    }

    if ((CurrentBranchPoints.Num() > 0 || ScopeStack.Num() > 0) && CurrentUserChoices.Num() == 0)
    {
        FClientConversationOptionEntry DefaultChoice;
        DefaultChoice.ChoiceText = UKismetTextLibrary::Conv_StringToText(FString(TEXT("Continue")));
        DefaultChoice.ChoiceType = EConversationChoiceType::UserChoiceAvailable;
        CurrentUserChoices.Add(DefaultChoice);
    }
}

const FConversationBranchPoint* UConversationInstance::FindBranchPointFromClientChoice(const FConversationChoiceReference& InChoice) const
{
    for (const FConversationBranchPoint& BranchPoint : CurrentBranchPoints)
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

    if (ScopeStack.Num() > 0 && ScopeStack[ScopeStack.Num() - 1].NodeReference.NodeGUID == TaskNode->GetNodeGuid())
        ScopeStack.RemoveAt(ScopeStack.Num() - 1);

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
