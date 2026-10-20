#include "pch.h"
#include "Abilities/FortGameplayAbility.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static Scanner::FCode ClientEndAbilityWeaponCall()
{
    static auto ClientEndAbilityWeaponCall = Scanner::CallTo(Scanner::Event(TEXT("ClientEndAbilityWeapon")).Get());

    return ClientEndAbilityWeaponCall;
}

static int32_t AbilityWeaponItemOffset()
{
    static auto AbilityWeaponItem = ClientEndAbilityWeaponCall().Last("mov dword ptr [*+0x*], 0xffffffff").Disp(TEXT("UFortGameplayAbility::AbilityWeaponItem"));

    return AbilityWeaponItem;
}

static int32_t AbilityWeaponOffset()
{
    static auto Lea = ClientEndAbilityWeaponCall().Find("lea *, [*+0x*]");
    static auto Add = ClientEndAbilityWeaponCall().Find("add *, 0x*");
    static auto AbilityWeapon = Add.Get() && (!Lea.Get() || Add.Get() < Lea.Get()) ? (int32_t)Add.Imm(TEXT("UFortGameplayAbility::AbilityWeaponActor")) : Lea.Disp(TEXT("UFortGameplayAbility::AbilityWeaponActor"));

    return AbilityWeapon;
}

static auto execEquipAbilityWeapon = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon"));

    if (!Function || !StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ClientEndAbilityWeapon")) || !Scanner::Native(TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon")).Is("ret*") || AbilityWeaponItemOffset() <= 0 || AbilityWeaponOffset() <= 0)
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked UFortGameplayAbility::execEquipAbilityWeapon, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&UFortGameplayAbility::execEquipAbilityWeapon);

    return true;
}();

UClass* UFortGameplayAbility::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbility"));

    return Class;
}

void UFortGameplayAbility::EquipAbilityWeapon(UFortWeaponItemDefinition* WeaponDefinition, int32 ItemLevel)
{
    auto Pawn = Cast<AFortPawn>(GetAvatarActorFromActorInfo());
    auto PlayerController = Pawn ? Cast<AFortPlayerControllerAthena>(Pawn->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WeaponDefinition || !WorldInventory || !Pawn->HasAuthority())
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(WeaponDefinition, 1, ItemLevel);
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
            GetAbilityWeaponItem() = Item;
    }

    GetAbilityWeapon() = Pawn->EquipWeaponDefinition(WeaponDefinition, ItemGuid);

    PlayerController->ClientEquipItem(ItemGuid, true);
}

void UFortGameplayAbility::execEquipAbilityWeapon(FFrame& Stack, void* const Z_Param__Result)
{
    UFortWeaponItemDefinition* WeaponDefinition = nullptr;
    int32 ItemLevel = 0;

    Stack.StepCompiledIn(&WeaponDefinition);
    Stack.StepCompiledIn(&ItemLevel);
    Stack.Code += !!Stack.Code;

    EquipAbilityWeapon(WeaponDefinition, ItemLevel);
}

TWeakObjectPtr<UFortWorldItem>& UFortGameplayAbility::GetAbilityWeaponItem()
{
    return *(TWeakObjectPtr<UFortWorldItem>*)((uint8_t*)this + AbilityWeaponItemOffset());
}

TWeakObjectPtr<AFortWeapon>& UFortGameplayAbility::GetAbilityWeapon()
{
    return *(TWeakObjectPtr<AFortWeapon>*)((uint8_t*)this + AbilityWeaponOffset());
}
