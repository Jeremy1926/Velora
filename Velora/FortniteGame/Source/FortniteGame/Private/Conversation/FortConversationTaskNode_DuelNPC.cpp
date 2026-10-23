#include "pch.h"
#include "Conversation/FortConversationTaskNode_DuelNPC.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "AI/FortAthenaAIBotController.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_DuelNPC::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_DuelNPC"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_DuelNPC::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    return FConversationTaskResult::AdvanceConversation();
}

FConversationTaskResult UFortConversationTaskNode_DuelNPC::ExecuteChallengeTaskNode(const FConversationContext& Context) const
{
    static auto NonParticipantClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/AI/NPCs/Base/BP_PlayerPawn_NonParticipant.BP_PlayerPawn_NonParticipant_C"));

    auto NPCActor = UFortConversationContextLibrary::GetParticipantActorNPC(Context);
    auto PlayerController = Cast<AFortPlayerController>(UFortConversationContextLibrary::GetParticipantActorPlayer(Context));

    if (!PlayerController)
        return FConversationTaskResult();

    if (NPCActor && NonParticipantClass && NPCActor->IsA(NonParticipantClass))
    {
        auto PlayerPawn = Cast<AFortPlayerPawn>(PlayerController->GetPawn());

        if (!PlayerPawn)
            return FConversationTaskResult();

        SetChallenger(NPCActor, PlayerPawn);
    }

    return ExecuteTaskNode_Implementation(Context);
}

void UFortConversationTaskNode_DuelNPC::SetChallenger(AActor* NonParticipantPawn, AFortPlayerPawn* Challenger) const
{
    static auto OnDied = AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("OnDied")));

    auto ChallengerProperty = NonParticipantPawn->GetClass()->FindPropertyByName(FName(TEXT("Challenger")));
    auto GE_DuelTag = NonParticipantPawn->GetClass()->FindPropertyByName(FName(TEXT("GE_DuelTag")));
    auto GEDuelHandle = NonParticipantPawn->GetClass()->FindPropertyByName(FName(TEXT("GEDuelHandle")));

    if (!ChallengerProperty || !GE_DuelTag || !GEDuelHandle)
        return;

    *ChallengerProperty->ContainerPtrToValuePtr<AFortPlayerPawn*>(NonParticipantPawn) = Challenger;

    FScriptDelegate ChallengerDied;
    ChallengerDied.BindUFunction(NonParticipantPawn, FName(TEXT("ChallengerDied")));
    OnDied->ContainerPtrToValuePtr<FMulticastScriptDelegate>(Challenger)->AddUnique(ChallengerDied);

    auto FortPawn = Cast<AFortPawn>(NonParticipantPawn);

    if (auto AbilitySystemComponent = FortPawn ? FortPawn->GetAbilitySystemComponent() : nullptr)
        *GEDuelHandle->ContainerPtrToValuePtr<FActiveGameplayEffectHandle>(NonParticipantPawn) = AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(*GE_DuelTag->ContainerPtrToValuePtr<UClass*>(NonParticipantPawn), 0.0f, FGameplayEffectContextHandle());
}

void UFortConversationTaskNode_DuelNPC::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    int32 InventoryCount = 0;
    auto NPCPawn = Cast<AFortPlayerPawn>(UFortConversationContextLibrary::GetParticipantActorNPC(Context));
    auto NPCController = NPCPawn ? Cast<AFortAthenaAIBotController>(NPCPawn->GetController()) : nullptr;
    auto Inventory = NPCController ? NPCController->GetInventory() : nullptr;

    if (Inventory)
    {
        for (auto Item : Inventory->GetInventory().GetItemInstances())
        {
            if (!Item || !Item->CanBeDropped() || !Item->GetNumInStack())
                continue;

            auto ItemDefinition = Item->GetItemDefinitionBP();

            if (!ItemDefinition)
                continue;

            FString ItemKey(TEXT("inventory_item_"));
            ItemKey += FString::FromInt(InventoryCount);

            FString ItemCountKey(TEXT("inventory_item_count_"));
            ItemCountKey += FString::FromInt(InventoryCount);

            InOutExtraData.Add({ ItemKey, UKismetSystemLibrary::GetPathName(ItemDefinition) });
            InOutExtraData.Add({ ItemCountKey, FString::FromInt(Item->GetNumInStack()) });
            InventoryCount++;
        }
    }

    InOutExtraData.Add({ FString(TEXT("inventory_count")), FString::FromInt(InventoryCount) });
}
