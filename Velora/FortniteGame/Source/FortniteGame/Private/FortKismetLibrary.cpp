#include "pch.h"
#include "FortKismetLibrary.h"
#include "Abilities/FortAbilitySet.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto PickLootDrops = []
{
    auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops"));
    auto Call = Exec.Find("call 0x*; lea rcx, [rsp+0x20]");
    auto Address = (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; call 0x*").Find("call 0x*")).Get();

    if (((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops")))->FindPropertyByName(FName(TEXT("WorldContextObject"))))
    {
        return DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), Address, (void*)+[](UObject* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
        {
            return UFortKismetLibrary::PickLootDrops(OutLootToDrop, TierGroupName, WorldLevel, ForcedLootTier);
        });
    }

    return DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), Address, (void*)&UFortKismetLibrary::PickLootDrops);
}();

static auto K2_SpawnPickupInWorld = []
{
    auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld"));
    auto Call = Exec.Find("call 0x*; mov rbx, [rsp+*]");

    return DetourCall(TEXT("UFortKismetLibrary::K2_SpawnPickupInWorld"), (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*").Find("call 0x*")).Get(), &UFortKismetLibrary::K2_SpawnPickupInWorld);
}();

static auto K2_RemoveItemFromPlayerByGuid = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid")) ? Detour(TEXT("UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid"), Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid")).Get(), &UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid) : nullptr;

static auto GiveItemToInventoryOwner = []
{
    auto Call = Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerControllerGameplay:StartGhostMode")).Find("lea rcx, [rsp+0x20]; mov rdx, *; call 0x*").Find("call 0x*");

    return Call.Get() && Call.Target() == Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner")).Get() ? DetourCall(TEXT("UFortKismetLibrary::GiveItemToInventoryOwner"), Call.Get(), &UFortKismetLibrary::GiveItemToInventoryOwner) : nullptr;
}();

FFortAbilitySetHandle UFortKismetLibrary::EquipFortAbilitySet(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor, UFortAbilitySet* AbilitySet, UObject* OverrideSourceObject)
{
    static auto EquipFortAbilitySet = Scanner::Function<FFortAbilitySetHandle* (*)(FFortAbilitySetHandle*, TScriptInterface<IAbilitySystemInterface>*, UFortAbilitySet*, UObject*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:EquipFortAbilitySet")).Find("mov r8, [rsp+0x*]").Call().Get(TEXT("UFortKismetLibrary::EquipFortAbilitySet")));

    FFortAbilitySetHandle Handle;

    EquipFortAbilitySet(&Handle, &AbilitySystemInterfaceActor, AbilitySet, OverrideSourceObject);

    return Handle;
}

void UFortKismetLibrary::GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer)
{
    if (auto PlayerController = Cast<AFortPlayerController>(InventoryOwner.GetObject()))
        K2_GiveItemToPlayer(PlayerController, ItemDefinition, NumberToGive);
}

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive)
{
    if (!PlayerController || !ItemDefinition)
        return;

    auto WorldInventory = PlayerController->GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, NumberToGive, ItemDefinition->GetMinLevel());
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;

    if (ItemEntry->GetParentInventory().IsValid())
        ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    if (!PlayerController->CanServerTryAddToQuickBar())
        return;

    for (auto Item : Inventory.ItemInstances)
    {
        if (Item->GetItemGuid() == ItemGuid)
            PlayerController->TryAddToQuickBar(Item);
    }
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval)
{
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory || !ItemDefinition)
        return 0;

    auto& ReplicatedEntries = WorldInventory->GetInventory().ReplicatedEntries;
    int32 AmountRemoved = 0;

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0 && (AmountToRemove < 0 || AmountRemoved < AmountToRemove); Index--)
    {
        if (ReplicatedEntries[Index].GetItemDefinition() != ItemDefinition)
            continue;

        const FGuid ItemGuid = ReplicatedEntries[Index].GetItemGuid();
        const int32 Count = AmountToRemove < 0 ? ReplicatedEntries[Index].GetCount() : FMath::Min(ReplicatedEntries[Index].GetCount(), AmountToRemove - AmountRemoved);

        if (PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval))
            AmountRemoved += Count;
    }

    return AmountRemoved;
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval)
{
    auto WorldInventory = PlayerController && ItemGuid.IsValid() ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return 0;

    for (auto& ItemEntry : WorldInventory->GetInventory().ReplicatedEntries)
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        const int32 Count = FMath::Min(ItemEntry.GetCount(), AmountToRemove);

        return PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval) ? Count : 0;
    }

    return 0;
}

void UFortKismetLibrary::BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
    static auto BroadcastSoundAtLocation = Scanner::Function<void (*)(UObject*, USoundBase*, const FVector&, float, float)>(Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastSoundAtLocation")).Get(TEXT("UFortKismetLibrary::BroadcastSoundAtLocation")));

    BroadcastSoundAtLocation(WorldContextObject, InSound, Location, VolumeMultiplier, PitchMultiplier);
}

bool UFortKismetLibrary::PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
{
    FName LootTierKey(NAME_None);
    int32 LootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, TierGroupName, WorldLevel, FGameplayTagContainer(), ForcedLootTier))
        return false;

    return UFortLootPackage::PickLootDrops(*OutLootToDrop, WorldLevel, LootTierKey, FGameplayTagContainer(), false, false);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss)
{
    static auto bHasToss = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld")))->FindPropertyByName(FName(TEXT("bToss"))) != nullptr;

    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World || !ItemDefinition || NumberToSpawn <= 0)
        return nullptr;

    FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, ItemDefinition->GetMinLevel());

    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

    if (ItemDefinition->GetItemType() == WeaponMelee || ItemDefinition->GetItemType() == WeaponRanged)
    {
        ItemEntry.SetDurability(ItemDefinition->GetMaxDurability(ItemEntry.GetLevel()));
        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
    }

    auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass());

    if (Pickup && (bToss || !bHasToss))
        Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, true);

    return Pickup;
}
