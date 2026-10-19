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

    return DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; call 0x*").Find("call 0x*")).Get(), &UFortKismetLibrary::PickLootDrops);
}();

static auto K2_SpawnPickupInWorld = []
{
    auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld"));
    auto Call = Exec.Find("call 0x*; mov rbx, [rsp+*]");

    return DetourCall(TEXT("UFortKismetLibrary::K2_SpawnPickupInWorld"), (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*").Find("call 0x*")).Get(), &UFortKismetLibrary::K2_SpawnPickupInWorld);
}();

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive)
{
    if (!PlayerController || !ItemDefinition)
        return;

    auto WorldInventory = PlayerController->GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, NumberToGive, ItemDefinition->GetMinLevel());

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;

    if (ItemEntry->GetParentInventory().IsValid())
        ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    PlayerController->AddItemToQuickBars(ItemDefinition, EFortQuickBars::Max_None, -1);
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval)
{
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory || !ItemDefinition)
        return 0;

    auto& ReplicatedEntries = WorldInventory->GetInventory().ReplicatedEntries;
    int32 AmountRemoved = 0;

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0 && AmountRemoved < AmountToRemove; Index--)
    {
        if (ReplicatedEntries[Index].GetItemDefinition() != ItemDefinition)
            continue;

        const FGuid ItemGuid = ReplicatedEntries[Index].GetItemGuid();
        const int32 Count = FMath::Min(ReplicatedEntries[Index].GetCount(), AmountToRemove - AmountRemoved);

        if (PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval))
            AmountRemoved += Count;
    }

    return AmountRemoved;
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

    if (ItemDefinition->GetItemType() == EFortItemType::WeaponMelee || ItemDefinition->GetItemType() == EFortItemType::WeaponRanged)
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
