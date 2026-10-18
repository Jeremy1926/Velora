#include "pch.h"
#include "FortKismetLibrary.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto PickLootDrops = DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops")).Find("call 0x*; lea rcx, [rsp+0x20]").Get(), &UFortKismetLibrary::PickLootDrops);

static auto K2_SpawnPickupInWorld = DetourCall(TEXT("UFortKismetLibrary::K2_SpawnPickupInWorld"), Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld")).Find("call 0x*; mov rbx, [rsp+*]").Get(), &UFortKismetLibrary::K2_SpawnPickupInWorld);

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive)
{
    if (!PlayerController || !ItemDefinition)
        return;

    auto WorldInventory = PlayerController->GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, NumberToGive, ItemDefinition->GetMinLevel());

    Inventory.MarkItemDirty(*ItemEntry);
    WorldInventory->HandleInventoryLocalUpdate();

    PlayerController->AddItemToQuickBars(ItemDefinition, EFortQuickBars::Max_None, -1);
}

void UFortKismetLibrary::BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
    static auto BroadcastSoundAtLocation = (void (*)(UObject*, USoundBase*, const FVector&, float, float))Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastSoundAtLocation")).Get();

    BroadcastSoundAtLocation(WorldContextObject, InSound, Location, VolumeMultiplier, PitchMultiplier);
}

bool UFortKismetLibrary::PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
{
    FName LootTierKey(NAME_None);
    int32 LootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, TierGroupName, WorldLevel, FGameplayTagContainer()))
        return false;

    auto bPicked = UFortLootPackage::PickLootDrops(*OutLootToDrop, WorldLevel, LootTierKey, FGameplayTagContainer(), false, false);

    UE_LOG(LogVelora, Log, TEXT("PickLootDrops %s picked %s with %d items"), *TierGroupName.ToString(), *LootTierKey.ToString(), OutLootToDrop->Num());

    return bPicked;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss)
{
    static auto bHasToss = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld")))->FindPropertyByName(FName(TEXT("bToss"))) != nullptr;

    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World || !ItemDefinition || NumberToSpawn <= 0)
        return nullptr;

    FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, ItemDefinition->GetMinLevel());

    if (ItemDefinition->GetItemType() == EFortItemType::WeaponMelee || ItemDefinition->GetItemType() == EFortItemType::WeaponRanged)
    {
        ItemEntry.SetDurability(ItemDefinition->GetMaxDurability(ItemEntry.Level));
        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.Level));
    }

    auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass());

    if (Pickup && (bToss || !bHasToss))
        Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, true);

    UE_LOG(LogVelora, Log, TEXT("K2_SpawnPickupInWorld spawned %s x%d"), *ItemDefinition->GetName(), NumberToSpawn);

    return Pickup;
}
