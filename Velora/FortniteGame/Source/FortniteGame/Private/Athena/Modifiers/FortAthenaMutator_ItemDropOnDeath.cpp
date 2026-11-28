#include "pch.h"
#include "Athena/Modifiers/FortAthenaMutator_ItemDropOnDeath.h"
#include "FortKismetLibrary.h"
#include "Items/FortInventory.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortPickup.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h"
#include "HAL/IConsoleManager.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FItemsToDropOnDeath::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ItemsToDropOnDeath"));

    return Struct;
}

UFortWorldItemDefinition* FItemsToDropOnDeath::GetItemToDrop() const
{
    static auto ItemToDrop = StaticStruct()->FindPropertyByName(FName(TEXT("ItemToDrop")));

    return *ItemToDrop->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

const FScalableFloat& FItemsToDropOnDeath::GetNumberToDrop() const
{
    static auto NumberToDrop = StaticStruct()->FindPropertyByName(FName(TEXT("NumberToDrop")));

    return *NumberToDrop->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UClass* AFortAthenaMutator_ItemDropOnDeath::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMutator_ItemDropOnDeath"));

    return Class;
}

void AFortAthenaMutator_ItemDropOnDeath::SpawnItems(AFortPawn* Pawn)
{
    DETOUR(AFortAthenaMutator_ItemDropOnDeath, SpawnItems, Anchor::Hook()
        .If(StaticClass())
        .String(TEXT("AFortAthenaMutator_ItemDropOnDeath::SpawnItems"))
        .Start()
        .Or()
        .If(StaticClass())
        .FieldUser(TEXT("/Script/FortniteGame.Default__FortAthenaMutator_ItemDropOnDeath"), "test byte ptr [*+%x], 0xfd", StaticClass() ? StaticClass()->FindPropertyByName(FName(TEXT("RespawnRequirements"))) : nullptr)
        .FieldFind("test byte ptr [*+%x], 0xfd", StaticClass() ? StaticClass()->FindPropertyByName(FName(TEXT("RespawnRequirements"))) : nullptr)
        .Find("call 0x*")
        .Target()
        .Inline());

    static auto DeathDropsEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.Pickup.DeathDropsEnabled"));
    static auto DropOnDeathSourceEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.Pickup.DropOnDeathSourceEnabled.ItemDropOnDeathMutator"));

    if ((DeathDropsEnabled && !DeathDropsEnabled->GetInt()) || (DropOnDeathSourceEnabled && !DropOnDeathSourceEnabled->GetInt()))
        return;

    const float PercentAmmoToDrop = GetPercentAmmoToDrop().GetValueAtLevel(0.0f);
    const float AmmoDespawnTime = GetAmmoDespawnTime().GetValueAtLevel(0.0f);
    const float AmmoStormDespawnTime = GetAmmoStormDespawnTime().GetValueAtLevel(0.0f);
    const int32 MaxAmmoTypeDrops = (int32)GetMaxAmmoTypeDrops().GetValueAtLevel(0.0f);
    const int32 MaxResourceTypeDrops = (int32)GetMaxResourceTypeDrops().GetValueAtLevel(0.0f);
    auto World = Pawn ? GetWorld() : nullptr;

    if (!World)
        return;

    static auto Ammo = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::Ammo")));
    static auto WorldResource = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WorldResource")));

    const FVector Location = Pawn->GetActorLocation();
    const FVector Direction = Pawn->GetActorRotation().Vector();
    auto Controller = PercentAmmoToDrop > 0.0f ? Pawn->GetController() : nullptr;
    auto InventoryOwner = Controller ? (IFortInventoryOwnerInterface*)Controller->GetInterfaceAddress(IFortInventoryOwnerInterface::StaticClass()) : nullptr;
    auto Inventory = InventoryOwner ? InventoryOwner->GetWorldInventory() : nullptr;

    if (Inventory)
    {
        for (auto Item : Inventory->GetInventory().GetItemInstances())
        {
            if (!Item || Item->GetType() != Ammo)
                continue;

            auto ItemDefinition = Cast<UFortWorldItemDefinition>(Item->GetItemDefinitionBP());
            const int32 NumberToDrop = ItemDefinition && Item->GetNumInStack() > 0 ? (int32)(Item->GetNumInStack() * PercentAmmoToDrop) : 0;

            if (NumberToDrop <= 0)
                continue;

            if (auto Pickup = UFortKismetLibrary::SpawnItemVariantPickupInWorld(World, ItemDefinition, NumberToDrop, Location, Direction, 0, true, true, false, -1, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, nullptr, true))
                Pickup->InitializeDespawnManually(AmmoDespawnTime, AmmoStormDespawnTime, false);
        }
    }

    TArray<const FItemsToDropOnDeath*> AmmoItems;
    TArray<const FItemsToDropOnDeath*> ResourceItems;
    TArray<const FItemsToDropOnDeath*> ItemsToSpawn;

    for (int32 Index = 0; Index < GetNumItemsToDrop(); Index++)
    {
        auto& ItemToDrop = GetItemsToDrop(Index);
        auto ItemDefinition = ItemToDrop.GetItemToDrop();

        if (ItemDefinition && ItemDefinition->GetItemType() == Ammo)
            AmmoItems.Add(&ItemToDrop);
        else if (ItemDefinition && ItemDefinition->GetItemType() == WorldResource)
            ResourceItems.Add(&ItemToDrop);
        else
            ItemsToSpawn.Add(&ItemToDrop);
    }

    for (auto [Items, MaxTypeDrops] : { std::pair(&AmmoItems, MaxAmmoTypeDrops), std::pair(&ResourceItems, MaxResourceTypeDrops) })
    {
        if (MaxTypeDrops < 0 || MaxTypeDrops >= Items->Num())
        {
            ItemsToSpawn.Append(*Items);
            continue;
        }

        for (int32 Drop = 0; Drop < MaxTypeDrops; Drop++)
        {
            const int32 Index = FMath::RandHelper(Items->Num());

            ItemsToSpawn.Add((*Items)[Index]);
            Items->RemoveAt(Index);
        }
    }

    for (auto ItemToDrop : ItemsToSpawn)
    {
        const int32 NumberToDrop = (int32)ItemToDrop->GetNumberToDrop().GetValueAtLevel(0.0f);

        if (!ItemToDrop->GetItemToDrop() || NumberToDrop <= 0)
            continue;

        if (auto Pickup = UFortKismetLibrary::SpawnItemVariantPickupInWorld(World, ItemToDrop->GetItemToDrop(), NumberToDrop, Location, Direction, 0, true, true, false, -1, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, nullptr, true))
            Pickup->InitializeDespawnManually(AmmoDespawnTime, AmmoStormDespawnTime, false);
    }

    for (auto& LootTierGroup : GetLootTierGroupsToDrop())
    {
        if (LootTierGroup == FName(NAME_None))
            continue;

        for (auto Pickup : UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTier(this, LootTierGroup, Location, 0, true, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset))
            Pickup->InitializeDespawnManually(AmmoDespawnTime, AmmoStormDespawnTime, false);
    }
}

const FScalableFloat& AFortAthenaMutator_ItemDropOnDeath::GetPercentAmmoToDrop() const
{
    static auto PercentAmmoToDrop = StaticClass()->FindPropertyByName(FName(TEXT("PercentAmmoToDrop")));

    return *PercentAmmoToDrop->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMutator_ItemDropOnDeath::GetAmmoDespawnTime() const
{
    static auto AmmoDespawnTime = StaticClass()->FindPropertyByName(FName(TEXT("AmmoDespawnTime")));

    return *AmmoDespawnTime->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMutator_ItemDropOnDeath::GetAmmoStormDespawnTime() const
{
    static auto AmmoStormDespawnTime = StaticClass()->FindPropertyByName(FName(TEXT("AmmoStormDespawnTime")));

    return *AmmoStormDespawnTime->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMutator_ItemDropOnDeath::GetMaxAmmoTypeDrops() const
{
    static auto MaxAmmoTypeDrops = StaticClass()->FindPropertyByName(FName(TEXT("MaxAmmoTypeDrops")));

    return *MaxAmmoTypeDrops->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMutator_ItemDropOnDeath::GetMaxResourceTypeDrops() const
{
    static auto MaxResourceTypeDrops = StaticClass()->FindPropertyByName(FName(TEXT("MaxResourceTypeDrops")));

    return *MaxResourceTypeDrops->ContainerPtrToValuePtr<FScalableFloat>(this);
}

int32 AFortAthenaMutator_ItemDropOnDeath::GetNumItemsToDrop() const
{
    static auto ItemsToDrop = StaticClass()->FindPropertyByName(FName(TEXT("ItemsToDrop")));

    return ItemsToDrop->ContainerPtrToValuePtr<TArray<uint8>>(this)->Num();
}

const FItemsToDropOnDeath& AFortAthenaMutator_ItemDropOnDeath::GetItemsToDrop(int32 Index) const
{
    static auto ItemsToDrop = (UArrayProperty*)StaticClass()->FindPropertyByName(FName(TEXT("ItemsToDrop")));
    static auto ElementSize = ItemsToDrop->GetInner()->GetSize();

    return *(const FItemsToDropOnDeath*)(ItemsToDrop->ContainerPtrToValuePtr<TArray<uint8>>(this)->GetData() + Index * ElementSize);
}

TArray<FName> AFortAthenaMutator_ItemDropOnDeath::GetLootTierGroupsToDrop() const
{
    static auto LootTierGroupsToDrop = StaticClass()->FindPropertyByName(FName(TEXT("LootTierGroupsToDrop")));

    TArray<FName> LootTierGroups;

    if (!LootTierGroupsToDrop)
        return LootTierGroups;

    auto& Groups = *LootTierGroupsToDrop->ContainerPtrToValuePtr<TArray<uint8>>(this);

    for (int32 Index = 0; Index < Groups.Num(); Index++)
        LootTierGroups.Add(TGameLayout<FName>::Load(Groups.GetData() + Index * TGameLayout<FName>::Size()));

    return LootTierGroups;
}
