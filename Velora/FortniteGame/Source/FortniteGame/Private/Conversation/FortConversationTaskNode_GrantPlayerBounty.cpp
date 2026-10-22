#include "pch.h"
#include "Conversation/FortConversationTaskNode_GrantPlayerBounty.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Athena/ControllerComponents/FortControllerComponent_TransientQuests.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Building/BuildingGameplayActorTransientQuest.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Player/FortPlayerController.h"
#include "Quests/FortQuestItemDefinition.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_GrantPlayerBounty::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GrantPlayerBounty"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_GrantPlayerBounty::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    static auto PlayerParticipantTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Conversation.Participant.Player")));
    static auto NPCParticipantTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Conversation.Participant.NPC")));
    static auto IsTargetedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Athena.Bounty.IsTargeted")));
    static auto TargetQuestToGrantPtr = StaticClass()->FindPropertyByName(FName(TEXT("TargetQuestToGrantPtr")));
    static auto ProtectorQuestToGrantPtr = StaticClass()->FindPropertyByName(FName(TEXT("ProtectorQuestToGrantPtr")));
    static auto TargetFilter = StaticClass()->FindPropertyByName(FName(TEXT("TargetFilter")));
    static auto Price = StaticClass()->FindPropertyByName(FName(TEXT("Price")));
    static auto ChanceToBeHuman = StaticClass()->FindPropertyByName(FName(TEXT("ChanceToBeHuman")));
    static auto MediumThreatRange = StaticClass()->FindPropertyByName(FName(TEXT("MediumThreatRange")));
    static auto HighThreatRange = StaticClass()->FindPropertyByName(FName(TEXT("HighThreatRange")));
    static auto ThreatRefreshRate = StaticClass()->FindPropertyByName(FName(TEXT("ThreatRefreshRate")));

    auto ParticipantActor = Context.GetParticipantActor(PlayerParticipantTag);
    auto ParticipantPawn = Cast<AFortPawn>(ParticipantActor);
    auto HunterController = Cast<AFortPlayerControllerAthena>(ParticipantPawn ? (AActor*)ParticipantPawn->GetController() : ParticipantActor);
    auto HunterPlayerState = HunterController ? Cast<AFortPlayerStateAthena>(HunterController->GetPlayerState()) : nullptr;
    auto GameMode = HunterPlayerState ? Cast<AFortGameModeAthena>(HunterController->GetWorld()->GetAuthGameMode()) : nullptr;

    if (!GameMode)
        return FConversationTaskResult::AbortConversation();

    TArray<AFortPlayerStateAthena*> HumanTargets;
    TArray<AFortPlayerStateAthena*> BotTargets;

    UFortControllerComponent_TransientQuests::GetPlayerBountyTargets(HunterController, *TargetFilter->ContainerPtrToValuePtr<FGameplayTagQuery>(this), HumanTargets, BotTargets);

    const bool bHumanTarget = HumanTargets.Num() && (!BotTargets.Num() || FMath::FRand() * 100.0f < ChanceToBeHuman->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));
    auto& Targets = bHumanTarget ? HumanTargets : BotTargets;

    if (!Targets.Num())
        return FConversationTaskResult::AbortConversation();

    auto TargetPlayerState = Targets[FMath::RandHelper(Targets.Num())];
    const int32 PriceValue = FMath::TruncToInt(Price->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));

    if (PriceValue > 0)
        UFortKismetLibrary::K2_RemoveItemFromPlayer(HunterController, GetResourceCurrency(), PriceValue, true);

    TArray<AFortPlayerStateAthena*> HunterPlayerStates;
    TArray<UFortControllerComponent_TransientQuests*> HunterComponents;
    TArray<UFortControllerComponent_TransientQuests*> ProtectorComponents;
    UFortControllerComponent_TransientQuests* TargetComponent = nullptr;

    for (auto PlayerController : GameMode->GetAlivePlayers())
    {
        auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;
        auto TransientQuests = PlayerState ? PlayerController->FindComponentByClass<UFortControllerComponent_TransientQuests>() : nullptr;

        if (!TransientQuests)
            continue;

        if (PlayerState == TargetPlayerState)
        {
            TargetComponent = TransientQuests;
        }
        else if (PlayerState->GetSquadId() == HunterPlayerState->GetSquadId())
        {
            HunterPlayerStates.Add(PlayerState);
            HunterComponents.Add(TransientQuests);
        }
        else if (PlayerState->GetSquadId() == TargetPlayerState->GetSquadId())
        {
            ProtectorComponents.Add(TransientQuests);
        }
    }

    if (auto TargetAbilitySystemComponent = TargetPlayerState->GetAbilitySystemComponent())
        TargetAbilitySystemComponent->UpdateTagMap(IsTargetedTag, 1);

    for (auto HunterComponent : HunterComponents)
    {
        HunterComponent->GetTrackedHunterBountyTarget() = TargetPlayerState;
        HunterComponent->GrantTransientQuest(GetQuestToGrantPtr());
    }

    for (auto ProtectorComponent : ProtectorComponents)
    {
        ProtectorComponent->GetTrackedProtectorBountyTarget() = TargetPlayerState;
        ProtectorComponent->GetTrackedBountyHunters() = HunterPlayerStates;
        ProtectorComponent->GrantTransientQuest(*ProtectorQuestToGrantPtr->ContainerPtrToValuePtr<TSoftObjectPtr<UFortQuestItemDefinition>>(this));
    }

    if (TargetComponent)
    {
        TargetComponent->GetTrackedBountyHunters() = HunterPlayerStates;
        TargetComponent->GrantTransientQuest(*TargetQuestToGrantPtr->ContainerPtrToValuePtr<TSoftObjectPtr<UFortQuestItemDefinition>>(this));
        TargetComponent->StartPlayerBountyThreatLevelUpdates(MediumThreatRange->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f), HighThreatRange->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f), ThreatRefreshRate->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));
    }

    if (auto TransientQuest = Cast<ABuildingGameplayActorTransientQuest>(Context.GetParticipantActor(NPCParticipantTag)))
        TransientQuest->OnBountyStarted(HunterPlayerStates);

    return FConversationTaskResult::AdvanceConversation();
}

EConversationRequirementResult UFortConversationTaskNode_GrantPlayerBounty::IsRequirementSatisfied_Implementation(const FConversationContext& Context) const
{
    static auto PlayerParticipantTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Conversation.Participant.Player")));
    static auto TargetFilter = StaticClass()->FindPropertyByName(FName(TEXT("TargetFilter")));
    static auto Price = StaticClass()->FindPropertyByName(FName(TEXT("Price")));

    auto ParticipantActor = Context.GetParticipantActor(PlayerParticipantTag);
    auto ParticipantPawn = Cast<AFortPawn>(ParticipantActor);
    auto HunterController = Cast<AFortPlayerControllerAthena>(ParticipantPawn ? (AActor*)ParticipantPawn->GetController() : ParticipantActor);
    auto TransientQuests = HunterController ? HunterController->FindComponentByClass<UFortControllerComponent_TransientQuests>() : nullptr;

    if (!TransientQuests || TransientQuests->GetTrackedHunterBountyTarget())
        return EConversationRequirementResult::FailedButVisible;

    TArray<AFortPlayerStateAthena*> HumanTargets;
    TArray<AFortPlayerStateAthena*> BotTargets;

    UFortControllerComponent_TransientQuests::GetPlayerBountyTargets(HunterController, *TargetFilter->ContainerPtrToValuePtr<FGameplayTagQuery>(this), HumanTargets, BotTargets);

    if (!HumanTargets.Num() && !BotTargets.Num())
        return EConversationRequirementResult::FailedButVisible;

    const int32 PriceValue = FMath::TruncToInt(Price->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));
    auto WorldInventory = HunterController->GetWorldInventory();
    int32 CurrencyCount = 0;

    if (PriceValue > 0 && WorldInventory)
    {
        for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
        {
            if (ItemEntry.GetItemDefinition() == (UFortItemDefinition*)GetResourceCurrency())
                CurrencyCount += ItemEntry.GetCount();
        }
    }

    return PriceValue > CurrencyCount ? EConversationRequirementResult::FailedButVisible : EConversationRequirementResult::Passed;
}
