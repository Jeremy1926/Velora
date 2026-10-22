#include "pch.h"
#include "Conversation/FortConversationTaskNode_UpgradeItem.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Weapons/FortWeapon.h"
#include "FortKismetLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_UpgradeItem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_UpgradeItem"));

    return Class;
}

const FWeaponUpgradeItemRow* UFortConversationTaskNode_UpgradeItem::GetUpgradedItemRow(const FConversationContext& Context) const
{
    static auto GetUpgradedItemRow = Anchor::Function<const FWeaponUpgradeItemRow* (*)(const UFortConversationTaskNode_UpgradeItem*, const FConversationContext&)>(TEXT("UFortConversationTaskNode_UpgradeItem::GetUpgradedItemRow"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Call(1)
        .Get();

    return GetUpgradedItemRow(this, Context);
}

TOptional<int32> UFortConversationTaskNode_UpgradeItem::GetUpgradePrice(const FWeaponUpgradeItemRow* UpgradedItemRow) const
{
    static auto GetUpgradePrice = Anchor::Function<TOptional<int32>* (*)(const UFortConversationTaskNode_UpgradeItem*, TOptional<int32>*, const FWeaponUpgradeItemRow*)>(TEXT("UFortConversationTaskNode_UpgradeItem::GetUpgradePrice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Call(2)
        .Get();

    TOptional<int32> UpgradePrice;
    GetUpgradePrice(this, &UpgradePrice, UpgradedItemRow);

    return UpgradePrice;
}

FConversationTaskResult UFortConversationTaskNode_UpgradeItem::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto CurrentWeapon = PlayerPawn ? PlayerPawn->GetCurrentWeapon() : nullptr;
    auto UpgradedItemRow = GetUpgradedItemRow(Context);
    auto UpgradedWeaponDef = UpgradedItemRow ? UpgradedItemRow->GetUpgradedWeaponDef() : nullptr;

    if (!CurrentWeapon || !UpgradedWeaponDef || !PayPrice(Context, GetUpgradePrice(UpgradedItemRow)))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(PlayerController, CurrentWeapon->GetItemEntryGuid(), 1, true);
    ABuildingItemWeaponUpgradeActor::StaticClass()->GetDefaultObject<ABuildingItemWeaponUpgradeActor>()->GrantOutput(PlayerController, UpgradedWeaponDef);

    return FConversationTaskResult::AdvanceConversation();
}
