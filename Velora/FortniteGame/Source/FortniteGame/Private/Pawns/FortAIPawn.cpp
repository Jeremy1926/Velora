#include "pch.h"
#include "Pawns/FortAIPawn.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWorldItemDefinition.h"
#include "Weapons/FortWeapon.h"
#include "FortKismetLibrary.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

static std::unordered_map<AFortAIPawn*, std::tuple<FWeakObjectPtr, float, FName>> LootDropData;

UClass* AFortAIPawn::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIPawn"));

    return Class;
}

void AFortAIPawn::HandleDeathNotifications()
{
    DETOUR(AFortAIPawn, HandleDeathNotifications, Anchor::Hook()
        .String(TEXT("Failed to find spawn group info on enemy pawn death for pawn %s. Associated Mission Name: %s. Group GUID is %s. Num active groups %i."))
        .Caller()
        .InVirtual(TEXT("/Script/FortniteGame.Default__FortAIPawn"), TEXT("AFortAIPawn::Destroyed"), TEXT("AFortAIPawn::HandleOutOfHealth"))
        .AtCall());

    DropLoot();

    Original();
}

void AFortAIPawn::SetLootDropData(float LootDropChance, FName WorldItemTierGroup)
{
    LootDropData.insert_or_assign(this, std::tuple{ FWeakObjectPtr(this), LootDropChance, WorldItemTierGroup });
}

void AFortAIPawn::DropLoot()
{
    auto Found = LootDropData.find(this);

    if (Found == LootDropData.end())
        return;

    auto [Pawn, LootDropChance, WorldItemTierGroup] = Found->second;

    LootDropData.erase(Found);

    if (!Pawn.HasSameIndexAndSerialNumber(FWeakObjectPtr(this)) || GetHasDroppedLoot() || WorldItemTierGroup == NAME_None || FMath::FRand() >= LootDropChance)
        return;

    SetHasDroppedLoot(true);

    TArray<FFortItemEntry> LootToDrop;

    if (!UFortKismetLibrary::PickLootDrops(&LootToDrop, WorldItemTierGroup, -1, -1))
        return;

    const FVector LootSpawnLocation = GetActorLocation();

    for (auto& ItemEntry : LootToDrop)
    {
        if (!Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition()))
            continue;

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, LootSpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::AI))
            Pickup->TossPickup(LootSpawnLocation, nullptr, 0, true);
    }
}

void AFortAIPawn::GiveAmmo(const int32 AmmoQuantity)
{
    if (auto Weapon = GetCurrentWeapon())
        Weapon->ModAmmo(AmmoQuantity);
}

void AFortAIPawn::execGiveAmmo(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAIPawn, execGiveAmmo, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortAIPawn:GiveAmmo"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortAIPawn:GiveAmmo"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortAIPawn:GiveAmmo")));

    int32 AmmoQuantity = 0;

    Stack.StepCompiledIn(&AmmoQuantity);
    Stack.Code += !!Stack.Code;

    GiveAmmo(AmmoQuantity);
}

void AFortAIPawn::DropAllItems(bool bSpawnPickups)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory || !HasAuthority())
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto& ReplicatedEntries = Inventory.GetReplicatedEntries();

    if (!ReplicatedEntries.Num())
        return;

    const FVector Location = GetActorLocation();

    for (auto& ItemEntry : ReplicatedEntries)
    {
        auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition());

        if (!bSpawnPickups || !ItemDefinition || ItemEntry.GetCount() <= 0)
            continue;

        FFortItemEntry DropItemEntry(ItemDefinition, ItemEntry.GetCount(), ItemEntry.GetLevel());
        DropItemEntry.GetLoadedAmmo() = ItemEntry.GetLoadedAmmo();
        DropItemEntry.GetDurability() = ItemEntry.GetDurability();

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), DropItemEntry, Location, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::AI))
            Pickup->TossPickup(Location, this, 0, true);
    }

    ReplicatedEntries.RemoveAt(0, ReplicatedEntries.Num());
    Inventory.MarkArrayDirty();
    WorldInventory->ForceNetUpdate();
    WorldInventory->HandleInventoryLocalUpdate();
}

void AFortAIPawn::execDropAllItems(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAIPawn, execDropAllItems, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortAIPawn:DropAllItems"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortAIPawn:DropAllItems"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortAIPawn:DropAllItems")));

    uint32 bSpawnPickups = 0;

    Stack.StepCompiledIn(&bSpawnPickups);
    Stack.Code += !!Stack.Code;

    DropAllItems(!!bSpawnPickups);
}

void AFortAIPawn::SpawnPickupInWorldWithLootTierAsync(FName LootTierName, const FVector& Position, int32 OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source)
{
    if (HasAuthority())
        UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTier(this, LootTierName, Position, OverrideMaxStackCount, bToss, SourceType, Source);
}

void AFortAIPawn::execSpawnPickupInWorldWithLootTierAsync(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAIPawn, execSpawnPickupInWorldWithLootTierAsync, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIPawn:SpawnPickupInWorldWithLootTierAsync")))
        .StubExec(TEXT("/Script/FortniteGame.FortAIPawn:SpawnPickupInWorldWithLootTierAsync"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortAIPawn:SpawnPickupInWorldWithLootTierAsync")));

    FName LootTierName(NAME_None);
    TGameValue<FVector> Position;
    int32 OverrideMaxStackCount = 0;
    uint32 bToss = 0;
    EFortPickupSourceTypeFlag SourceType = EFortPickupSourceTypeFlag::Other;
    EFortPickupSpawnSource Source = EFortPickupSpawnSource::Unset;

    Stack.StepCompiledIn(&LootTierName);
    Stack.StepCompiledIn(&Position);
    Stack.StepCompiledIn(&OverrideMaxStackCount);
    Stack.StepCompiledIn(&bToss);
    Stack.StepCompiledIn(&SourceType);
    Stack.StepCompiledIn(&Source);
    Stack.Code += !!Stack.Code;

    SpawnPickupInWorldWithLootTierAsync(LootTierName, Position.Get(), OverrideMaxStackCount, !!bToss, SourceType, Source);
}

AFortInventory* AFortAIPawn::GetWorldInventory() const
{
    static auto Inventory = StaticClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}

bool AFortAIPawn::GetHasDroppedLoot() const
{
    static auto bHasDroppedLoot = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bHasDroppedLoot")));

    return bHasDroppedLoot->GetPropertyValue_InContainer(this);
}

void AFortAIPawn::SetHasDroppedLoot(bool bInHasDroppedLoot)
{
    static auto bHasDroppedLoot = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bHasDroppedLoot")));

    bHasDroppedLoot->SetPropertyValue_InContainer(this, bInHasDroppedLoot);
}

AFortPawn* AFortAIPawn::GetTetheredFollower() const
{
    static auto TetheredFollower = GetClass()->FindPropertyByName(FName(TEXT("TetheredFollower")));

    return *TetheredFollower->ContainerPtrToValuePtr<AFortPawn*>(this);
}
