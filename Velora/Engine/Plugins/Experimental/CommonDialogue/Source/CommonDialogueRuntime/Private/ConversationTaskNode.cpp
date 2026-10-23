#include "pch.h"
#include "ConversationTaskNode.h"
#include "ConversationChoiceNode.h"
#include "ConversationInstance.h"
#include "ConversationLinkNode.h"
#include "ConversationParticipantComponent.h"
#include "ConversationRegistry.h"
#include "ConversationRequirementNode.h"
#include "ConversationSideEffectNode.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortConversationTaskNode_ApplyAbility.h"
#include "Conversation/FortConversationTaskNode_ApplyDataDrivenEffect.h"
#include "Conversation/FortConversationTaskNode_DataDrivenService.h"
#include "Conversation/FortConversationTaskNode_DuelNPC.h"
#include "Conversation/FortConversationTaskNode_GrantPlayerBounty.h"
#include "Conversation/FortConversationTaskNode_GiveGift.h"
#include "Conversation/FortConversationTaskNode_GrantQuest.h"
#include "Conversation/FortConversationTaskNode_HireNPC.h"
#include "Conversation/FortConversationTaskNode_RevealDisguise.h"
#include "Conversation/FortConversationTaskNode_SellItem.h"
#include "Conversation/FortConversationTaskNode_ShowFutureStormCircle.h"
#include "Conversation/FortConversationTaskNode_UpgradeItem.h"
#include "Conversation/FortConversationTaskNode_Speech.h"
#include "Conversation/FortConversationTaskNode_SupplyDrop.h"
#include "Components/FortPawnComponent_Disguise.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
#include "Templates/TypeCompatibleBytes.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UConversationTaskNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode"));

    return Class;
}

const TArray<UConversationSubNode*>& UConversationTaskNode::GetSubNodes() const
{
    static auto SubNodes = StaticClass()->FindPropertyByName(FName(TEXT("SubNodes")));

    return *SubNodes->ContainerPtrToValuePtr<TArray<UConversationSubNode*>>(this);
}

EConversationRequirementResult UConversationTaskNode::MergeRequirements(EConversationRequirementResult CurrentResult, EConversationRequirementResult MergeResult)
{
    return (int64)MergeResult > (int64)CurrentResult ? MergeResult : CurrentResult;
}

EConversationRequirementResult UConversationTaskNode::CheckRequirements(const FConversationContext& InContext) const
{
    FConversationContext Context = InContext.CreateChildContext(this);
    EConversationRequirementResult FinalRequirementResult = EConversationRequirementResult::Passed;
    UWorld* World = Context.GetWorld();

    for (UConversationSubNode* SubNode : GetSubNodes())
    {
        auto RequirementNode = Cast<UConversationRequirementNode>(SubNode);

        if (!RequirementNode)
            continue;

        auto& EvalWorldContextObj = RequirementNode->GetEvalWorldContextObj();
        auto PreviousEvalWorldContextObj = EvalWorldContextObj;
        EvalWorldContextObj = (UObject*)World;

        FinalRequirementResult = MergeRequirements(FinalRequirementResult, RequirementNode->IsRequirementSatisfied(Context));

        EvalWorldContextObj = PreviousEvalWorldContextObj;

        if (FinalRequirementResult == EConversationRequirementResult::FailedAndHidden)
            break;
    }

    if (FinalRequirementResult != EConversationRequirementResult::FailedAndHidden)
    {
        auto& EvalWorldContextObj = GetEvalWorldContextObj();
        auto PreviousEvalWorldContextObj = EvalWorldContextObj;
        EvalWorldContextObj = (UObject*)World;

        FinalRequirementResult = MergeRequirements(FinalRequirementResult, IsRequirementSatisfied(Context));

        EvalWorldContextObj = PreviousEvalWorldContextObj;
    }

    return FinalRequirementResult;
}

EConversationRequirementResult UConversationTaskNode::IsRequirementSatisfied(const FConversationContext& Context) const
{
    static auto IsRequirementSatisfied_Implementation = Anchor::VirtualFunction<EConversationRequirementResult (*)(const UConversationTaskNode*, const FConversationContext&)>(TEXT("UConversationTaskNode::IsRequirementSatisfied_Implementation"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .Get();

    if (auto SellItem = Cast<UFortConversationTaskNode_SellItem>(this))
        return SellItem->IsRequirementSatisfied_Implementation(Context);

    if (auto GrantPlayerBounty = Cast<UFortConversationTaskNode_GrantPlayerBounty>(this))
        return GrantPlayerBounty->IsRequirementSatisfied_Implementation(Context);

    return IsRequirementSatisfied_Implementation(this, Context);
}

FConversationTaskResult UConversationTaskNode::ExecuteTaskNode(const FConversationContext& Context) const
{
    static auto ExecuteTaskNode_Implementation = Anchor::VirtualFunction<FConversationTaskResult* (*)(const UConversationTaskNode*, FConversationTaskResult*, const FConversationContext&)>(TEXT("UConversationTaskNode::ExecuteTaskNode_Implementation"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:ExecuteTaskNode"))
        .Find("call [*+0x*]")
        .Get();

    for (auto Struct = (UStruct*)GetClass(); Struct; Struct = Struct->GetSuperStruct())
    {
        const FName ClassName = Struct->GetFName();

        if (ClassName == FName(TEXT("Speech_C")))
            return ((const UFortConversationTaskNode_Speech*)this)->ExecuteSpeechTaskNode(Context);

        if (ClassName == FName(TEXT("Back_C")))
            return UConversationContextHelpers::ReturnToLastClientChoice(Context);

        if (ClassName == FName(TEXT("Repeat_C")))
            return UConversationContextHelpers::ReturnToCurrentClientChoice(Context);

        if (ClassName == FName(TEXT("Continue_C")))
            return UConversationContextHelpers::AdvanceConversation(Context);

        if (ClassName == FName(TEXT("ChallengeNPC_C")))
            return ((const UFortConversationTaskNode_DuelNPC*)this)->ExecuteChallengeTaskNode(Context);

        if (ClassName == FName(TEXT("DisguiseNPC_C")))
        {
            auto NPCActor = UFortConversationContextLibrary::GetParticipantActorNPC(Context);
            auto Disguise = NPCActor ? NPCActor->FindComponentByClass<UFortPawnComponent_Disguise>() : nullptr;

            if (Disguise)
                Disguise->RevealDisguise(nullptr);

            break;
        }
    }

    if (auto SellItem = Cast<UFortConversationTaskNode_SellItem>(this))
        return SellItem->ExecuteTaskNode_Implementation(Context);

    if (auto UpgradeItem = Cast<UFortConversationTaskNode_UpgradeItem>(this))
        return UpgradeItem->ExecuteTaskNode_Implementation(Context);

    if (auto HireNPC = Cast<UFortConversationTaskNode_HireNPC>(this))
        return HireNPC->ExecuteTaskNode_Implementation(Context);

    if (auto DuelNPC = Cast<UFortConversationTaskNode_DuelNPC>(this))
        return DuelNPC->ExecuteTaskNode_Implementation(Context);

    if (auto GrantPlayerBounty = Cast<UFortConversationTaskNode_GrantPlayerBounty>(this))
        return GrantPlayerBounty->ExecuteTaskNode_Implementation(Context);

    if (auto GrantQuest = Cast<UFortConversationTaskNode_GrantQuest>(this))
        return GrantQuest->ExecuteTaskNode_Implementation(Context);

    if (auto GiveGift = Cast<UFortConversationTaskNode_GiveGift>(this))
        return GiveGift->ExecuteTaskNode_Implementation(Context);

    if (auto ApplyAbility = Cast<UFortConversationTaskNode_ApplyAbility>(this))
        return ApplyAbility->ExecuteTaskNode_Implementation(Context);

    if (auto ShowFutureStormCircle = Cast<UFortConversationTaskNode_ShowFutureStormCircle>(this))
        return ShowFutureStormCircle->ExecuteTaskNode_Implementation(Context);

    if (auto DataDrivenService = Cast<UFortConversationTaskNode_DataDrivenService>(this))
        return DataDrivenService->ExecuteTaskNode_Implementation(Context);

    if (auto RevealDisguise = Cast<UFortConversationTaskNode_RevealDisguise>(this))
        return RevealDisguise->ExecuteTaskNode_Implementation(Context);

    if (auto SupplyDrop = Cast<UFortConversationTaskNode_SupplyDrop>(this))
        return SupplyDrop->ExecuteTaskNode_Implementation(Context);

    if (auto ApplyDataDrivenEffect = Cast<UFortConversationTaskNode_ApplyDataDrivenEffect>(this))
        return ApplyDataDrivenEffect->ExecuteTaskNode_Implementation(Context);

    TAlignedBytes<sizeof(FConversationTaskResult), alignof(FConversationTaskResult)> ResultBytes;
    auto NativeResult = (FConversationTaskResult*)&ResultBytes;
    ExecuteTaskNode_Implementation(this, NativeResult, Context);

    FConversationTaskResult Result = *NativeResult;
    NativeResult->~FConversationTaskResult();

    if (Result.Type == EConversationTaskResultType::Invalid)
        Result.Type = EConversationTaskResultType::AdvanceConversation;

    return Result;
}

FConversationTaskResult UConversationTaskNode::ExecuteTaskNodeWithSideEffects(const FConversationContext& InContext) const
{
    auto& EvalWorldContextObj = GetEvalWorldContextObj();
    auto PreviousEvalWorldContextObj = EvalWorldContextObj;
    EvalWorldContextObj = (UObject*)InContext.GetWorld();

    FConversationTaskResult Result = ExecuteTaskNode(InContext);

    if (Result.CanConversationContinue())
    {
        for (UConversationSubNode* SubNode : GetSubNodes())
        {
            if (auto SideEffectNode = Cast<UConversationSideEffectNode>(SubNode))
                SideEffectNode->CauseSideEffect(InContext);
        }

        const FConversationParticipants Participants = InContext.GetParticipantsCopy();

        for (const FConversationParticipantEntry& ParticipantEntry : Participants.List)
        {
            auto Component = ParticipantEntry.GetParticipantComponent();
            auto Owner = Component ? Component->GetOwner() : nullptr;

            if (Owner && Owner->GetRemoteRole() == ROLE_AutonomousProxy)
                Component->ServerNotifyExecuteTaskAndSideEffects(InContext.GetCurrentNodeHandle());
        }
    }

    EvalWorldContextObj = PreviousEvalWorldContextObj;

    return Result;
}

void UConversationTaskNode::GenerateChoicesForDestinations(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext, const TArray<FGuid>& CandidateDestinations)
{
    UWorld* World = InContext.GetWorld();

    for (const FGuid& DestinationGUID : CandidateDestinations)
    {
        auto DestinationTaskNode = InContext.ConversationRegistry ? Cast<UConversationTaskNode>(InContext.ConversationRegistry->GetRuntimeNodeFromGUID(DestinationGUID)) : nullptr;

        if (!DestinationTaskNode)
            continue;

        auto& EvalWorldContextObj = DestinationTaskNode->GetEvalWorldContextObj();
        auto PreviousEvalWorldContextObj = EvalWorldContextObj;
        EvalWorldContextObj = (UObject*)World;

        FConversationContext DestinationContext = InContext.CreateChildContext(DestinationTaskNode);
        const int32 StartingNumber = BranchBuilder.Num();

        DestinationTaskNode->GatherChoices(BranchBuilder, DestinationContext);

        if (BranchBuilder.Num() == StartingNumber && DestinationTaskNode->CheckRequirements(InContext) == EConversationRequirementResult::Passed)
        {
            FClientConversationOptionEntry DefaultChoice;
            DefaultChoice.ChoiceReference.NodeReference = DestinationGUID;
            DefaultChoice.ChoiceType = EConversationChoiceType::ServerOnly;

            BranchBuilder.AddChoice(DestinationContext, DefaultChoice);
        }

        EvalWorldContextObj = PreviousEvalWorldContextObj;
    }
}

void UConversationTaskNode::GatherChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const
{
    if (auto LinkNode = Cast<UConversationLinkNode>(this))
    {
        LinkNode->GatherChoices(BranchBuilder, Context);
        return;
    }

    GatherStaticChoices(BranchBuilder, Context);
    GatherDynamicChoices(BranchBuilder, Context);
}

void UConversationTaskNode::GatherStaticChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext) const
{
    UWorld* World = InContext.GetWorld();
    const EConversationRequirementResult RequirementResult = CheckRequirements(InContext);

    if (RequirementResult >= EConversationRequirementResult::FailedAndHidden)
        return;

    for (UConversationSubNode* SubNode : GetSubNodes())
    {
        auto ChoiceNode = Cast<UConversationChoiceNode>(SubNode);

        if (!ChoiceNode)
            continue;

        auto& EvalWorldContextObj = ChoiceNode->GetEvalWorldContextObj();
        auto PreviousEvalWorldContextObj = EvalWorldContextObj;
        EvalWorldContextObj = (UObject*)World;

        FClientConversationOptionEntry Choice;
        Choice.SetChoiceAvailable(RequirementResult == EConversationRequirementResult::Passed);

        if (ChoiceNode->GenerateChoice(InContext, Choice))
        {
            GatherStaticExtraData(InContext, Choice.ExtraData);
            BranchBuilder.AddChoice(InContext, Choice);
        }

        EvalWorldContextObj = PreviousEvalWorldContextObj;
    }
}

void UConversationTaskNode::GatherDynamicChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext) const
{
    if (auto SellItem = Cast<UFortConversationTaskNode_SellItem>(this))
        SellItem->GatherDynamicChoices(BranchBuilder, InContext);
}

void UConversationTaskNode::GatherStaticExtraData(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    static auto GatherStaticExtraData = Anchor::VirtualFunction<void (*)(const UConversationTaskNode*, const FConversationContext&, TArray<FConversationNodeParameterPair>&)>(TEXT("UConversationTaskNode::GatherStaticExtraData"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:GatherStaticExtraData"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode"), -0x18)
        .Get();

    GatherStaticExtraData(this, Context, InOutExtraData);
}
