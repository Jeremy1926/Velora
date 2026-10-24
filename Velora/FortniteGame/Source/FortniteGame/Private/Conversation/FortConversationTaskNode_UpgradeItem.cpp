#include "pch.h"
#include "Conversation/FortConversationTaskNode_UpgradeItem.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Weapons/FortWeapon.h"
#include "FortKismetLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
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
    static auto GetUpgradedItemRow = Anchor::Optional<const FWeaponUpgradeItemRow* (*)(const UFortConversationTaskNode_UpgradeItem*, const FConversationContext&)>(TEXT("UFortConversationTaskNode_UpgradeItem::GetUpgradedItemRow"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Find("call 0x*; test rax, rax; jz 0x*; mov r8, rax; lea rdx, [rsp+0x*]")
        .Target()
        .Get();

    if (GetUpgradedItemRow)
        return GetUpgradedItemRow(this, Context);

    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto CurrentWeapon = PlayerPawn ? PlayerPawn->GetCurrentWeapon() : nullptr;
    auto WeaponData = CurrentWeapon ? CurrentWeapon->GetWeaponData() : nullptr;

    return WeaponData ? ABuildingItemWeaponUpgradeActor::StaticClass()->GetDefaultObject<ABuildingItemWeaponUpgradeActor>()->GetItemRow(WeaponData, EFortWeaponUpgradeDirection::Vertical) : nullptr;
}

TOptional<int32> UFortConversationTaskNode_UpgradeItem::GetUpgradePrice(const FConversationContext& Context, const FWeaponUpgradeItemRow* UpgradedItemRow) const
{
    static auto GetUpgradePriceForContext = Anchor::Optional<TOptional<int32>* (*)(const UFortConversationTaskNode_UpgradeItem*, TOptional<int32>*, const FConversationContext&, const FWeaponUpgradeItemRow*)>(TEXT("UFortConversationTaskNode_UpgradeItem::GetUpgradePrice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Find("mov r9, rax; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    TOptional<int32> UpgradePrice;

    if (GetUpgradePriceForContext)
    {
        GetUpgradePriceForContext(this, &UpgradePrice, Context, UpgradedItemRow);

        return UpgradePrice;
    }

    static auto GetUpgradePrice = Anchor::Optional<TOptional<int32>* (*)(const UFortConversationTaskNode_UpgradeItem*, TOptional<int32>*, const FWeaponUpgradeItemRow*)>(TEXT("UFortConversationTaskNode_UpgradeItem::GetUpgradePrice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Find("mov r8, rax; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (GetUpgradePrice)
    {
        GetUpgradePrice(this, &UpgradePrice, UpgradedItemRow);

        return UpgradePrice;
    }

    static auto PricingKey = FName(TEXT("UpgradeItem"));

    return UpgradedItemRow ? FindServicePrice(PricingKey, (int32)UpgradedItemRow->GetUpgradedWeaponDef()->GetRarity()) : UpgradePrice;
}

FConversationTaskResult UFortConversationTaskNode_UpgradeItem::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto CurrentWeapon = PlayerPawn ? PlayerPawn->GetCurrentWeapon() : nullptr;
    auto UpgradedItemRow = GetUpgradedItemRow(Context);
    auto UpgradedWeaponDef = UpgradedItemRow ? UpgradedItemRow->GetUpgradedWeaponDef() : nullptr;

    if (!CurrentWeapon || !UpgradedWeaponDef || !PayPrice(Context, GetUpgradePrice(Context, UpgradedItemRow)))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(PlayerController, CurrentWeapon->GetItemEntryGuid(), 1, true);
    ABuildingItemWeaponUpgradeActor::StaticClass()->GetDefaultObject<ABuildingItemWeaponUpgradeActor>()->GrantOutput(PlayerController, UpgradedWeaponDef);

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_UpgradeItem::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    auto UpgradedItemRow = GetUpgradedItemRow(Context);
    auto UpgradedWeaponDef = UpgradedItemRow ? UpgradedItemRow->GetUpgradedWeaponDef() : nullptr;

    if (!UpgradedWeaponDef)
    {
        static auto WeaponUpgradeInteractionResult = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EFortWeaponUpgradeInteractionResult"));

        InOutExtraData.Add({ FString(TEXT("cannotupgradereason")), WeaponUpgradeInteractionResult ? WeaponUpgradeInteractionResult->GetNameStringByValue((int64)EFortWeaponUpgradeInteractionResult::CannotUpgrade) : FString() });
        return;
    }

    InOutExtraData.Add({ FString(TEXT("upgradetoitem")), UKismetSystemLibrary::GetPathName(UpgradedWeaponDef) });

    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto CurrentWeapon = PlayerPawn ? PlayerPawn->GetCurrentWeapon() : nullptr;
    auto WeaponData = CurrentWeapon ? CurrentWeapon->GetWeaponData() : nullptr;

    if (WeaponData)
        InOutExtraData.Add({ FString(TEXT("currentitem")), UKismetSystemLibrary::GetPathName(WeaponData) });

    const TOptional<int32> UpgradePrice = GetUpgradePrice(Context, UpgradedItemRow);
    InOutExtraData.Add({ FString(TEXT("cost")), FString::FromInt(UpgradePrice.IsSet() ? UpgradePrice.GetValue() : 99999999) });
}
