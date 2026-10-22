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
#include "Detour.h"

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

    if (auto WeaponStats = WeaponDefinition->GetWeaponStats<FFortBaseWeaponStats>())
        ItemEntry->SetLoadedAmmo(WeaponDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry->GetLevel()));

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    FGuid TrackerGuid;

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            GetAbilityWeaponItem() = Item;
            TrackerGuid = Item->GetTrackerGuid();
        }
    }

    GetAbilityWeapon() = Pawn->EquipWeaponDefinition(WeaponDefinition, ItemGuid, TrackerGuid);
}

void UFortGameplayAbility::execEquipAbilityWeapon(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameplayAbility, execEquipAbilityWeapon, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ClientEndAbilityWeapon")))
        .Native(TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon"))
        .Is("ret*")
        .Optional()
        .If([]
        {
            auto AbilityWeaponItem = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponItem"))
                .Event(TEXT("ClientEndAbilityWeapon"))
                .CallSite()
                .Last("mov dword ptr [*+0x*], 0xffffffff")
                .Disp();

            return AbilityWeaponItem > 0 && Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponActor"))
                .Event(TEXT("ClientEndAbilityWeapon"))
                .CallSite()
                .FirstOf("add *, 0x*", "lea *, [*+0x*]")
                .Get() > 0;
        })
        .InExec(TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon")));

    UFortWeaponItemDefinition* WeaponDefinition = nullptr;
    int32 ItemLevel = 0;

    Stack.StepCompiledIn(&WeaponDefinition);
    Stack.StepCompiledIn(&ItemLevel);
    Stack.Code += !!Stack.Code;

    EquipAbilityWeapon(WeaponDefinition, ItemLevel);
}

TWeakObjectPtr<UFortWorldItem>& UFortGameplayAbility::GetAbilityWeaponItem()
{
    static auto AbilityWeaponItem = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponItem"))
        .Event(TEXT("ClientEndAbilityWeapon"))
        .CallSite()
        .Last("mov dword ptr [*+0x*], 0xffffffff")
        .Disp();

    return *(TWeakObjectPtr<UFortWorldItem>*)((uint8_t*)this + AbilityWeaponItem);
}

TWeakObjectPtr<AFortWeapon>& UFortGameplayAbility::GetAbilityWeapon()
{
    static auto AbilityWeapon = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponActor"))
        .Event(TEXT("ClientEndAbilityWeapon"))
        .CallSite()
        .FirstOf("add *, 0x*", "lea *, [*+0x*]")
        .Get();

    return *(TWeakObjectPtr<AFortWeapon>*)((uint8_t*)this + AbilityWeapon);
}
