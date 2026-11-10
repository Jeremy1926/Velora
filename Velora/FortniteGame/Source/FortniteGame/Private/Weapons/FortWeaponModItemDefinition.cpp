#include "pch.h"
#include "Weapons/FortWeaponModItemDefinition.h"
#include "Weapons/FortWeapon.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Items/FortWorldItem.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Engine/World.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FFortSavedWeaponModSlot::FFortSavedWeaponModSlot(const FFortWeaponModSlot& InSlot)
    : WeaponModTemplateID(InSlot.WeaponMod ? InSlot.WeaponMod->GetPersistentName() : FString())
    , bIsDynamic(InSlot.bIsDynamic)
{
}

UScriptStruct* FFortWeaponModSetRow::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponModSetRow"));

    return Struct;
}

FGameplayTag FFortWeaponModSetRow::GetBucketTag() const
{
    static auto BucketTag = StaticStruct()->FindPropertyByName(FName(TEXT("BucketTag")));

    return BucketTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

UFortWeaponModSetData* FFortWeaponModSetRow::GetSet() const
{
    static auto Set = StaticStruct()->FindPropertyByName(FName(TEXT("Set")));

    return *Set->ContainerPtrToValuePtr<UFortWeaponModSetData*>(this);
}

float FFortWeaponModSetRow::GetRandomWeight() const
{
    static auto RandomWeight = StaticStruct()->FindPropertyByName(FName(TEXT("RandomWeight")));

    return *RandomWeight->ContainerPtrToValuePtr<float>(this);
}

UClass* UFortWeaponModItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponModItemDefinition"));

    return Class;
}

FGameplayTag UFortWeaponModItemDefinition::GetModSlot() const
{
    static auto ModSlot = StaticClass()->FindPropertyByName(FName(TEXT("ModSlot")));

    return ModSlot->GetPropertyValue_InContainer<FGameplayTag>(this);
}

bool UFortWeaponModItemDefinition::CanBeUsedByWeaponDefinition(const UFortWeaponItemDefinition* WeaponDefinition) const
{
    static auto CanBeUsedByWeaponDefinition = Anchor::Function<bool (*)(const UFortWeaponModItemDefinition*, const UFortWeaponItemDefinition*)>(TEXT("UFortWeaponModItemDefinition::CanBeUsedByWeaponDefinition"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:ModAllowedOnWeaponDefinition"))
        .Find("test rcx, rcx; jz 0x*; call 0x*; mov *, al")
        .Find("call 0x*")
        .Target()
        .Get();

    return CanBeUsedByWeaponDefinition(this, WeaponDefinition);
}

UClass* UFortWeaponModSetData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponModSetData"));

    return Class;
}

const TArray<FFortModSetModEntry>* UFortWeaponModSetData::GetModsForRarity(EFortRarity Rarity) const
{
    static auto CommonRarityMods = StaticClass()->FindPropertyByName(FName(TEXT("CommonRarityMods")));
    static auto UncommonRarityMods = StaticClass()->FindPropertyByName(FName(TEXT("UncommonRarityMods")));
    static auto RareRarityMods = StaticClass()->FindPropertyByName(FName(TEXT("RareRarityMods")));
    static auto EpicRarityMods = StaticClass()->FindPropertyByName(FName(TEXT("EpicRarityMods")));
    static auto LegendaryRarityMods = StaticClass()->FindPropertyByName(FName(TEXT("LegendaryRarityMods"))) ? StaticClass()->FindPropertyByName(FName(TEXT("LegendaryRarityMods"))) : StaticClass()->FindPropertyByName(FName(TEXT("LengdaryRarityMods")));

    switch ((uint8)Rarity)
    {
    case 0:
        return CommonRarityMods->ContainerPtrToValuePtr<TArray<FFortModSetModEntry>>(this);
    case 1:
        return UncommonRarityMods->ContainerPtrToValuePtr<TArray<FFortModSetModEntry>>(this);
    case 2:
        return RareRarityMods->ContainerPtrToValuePtr<TArray<FFortModSetModEntry>>(this);
    case 3:
        return EpicRarityMods->ContainerPtrToValuePtr<TArray<FFortModSetModEntry>>(this);
    case 4:
        return LegendaryRarityMods->ContainerPtrToValuePtr<TArray<FFortModSetModEntry>>(this);
    default:
        return nullptr;
    }
}

bool UFortWeaponModSetData::ApplyModSetToItemEntry(FFortItemEntry& ItemEntry) const
{
    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry.GetItemDefinition());

    if (!WeaponItemDefinition)
        return false;

    auto Mods = GetModsForRarity(WeaponItemDefinition->GetRarity());

    if (!Mods)
        return false;

    WeaponItemDefinition->SetDefaultWeaponModSlotsIfNeeded(ItemEntry);

    TArray<FFortWeaponModSlot> WeaponModSlots = ItemEntry.GetWeaponModSlots();

    for (auto& ModEntry : *Mods)
    {
        if (!ModEntry.ModDefinition || !ModEntry.ModDefinition->CanBeUsedByWeaponDefinition(WeaponItemDefinition))
            continue;

        int32 SlotIndex = 0;

        while (SlotIndex < WeaponModSlots.Num() && (!WeaponModSlots[SlotIndex].WeaponMod || !(WeaponModSlots[SlotIndex].WeaponMod->GetModSlot() == ModEntry.ModDefinition->GetModSlot())))
        {
            SlotIndex++;
        }

        if (SlotIndex < WeaponModSlots.Num())
        {
            WeaponModSlots[SlotIndex] = FFortWeaponModSlot{ ModEntry.ModDefinition, true };
        }
        else
        {
            WeaponModSlots.Add(FFortWeaponModSlot{ ModEntry.ModDefinition, true });
        }
    }

    ItemEntry.SetWeaponModSlots(WeaponModSlots);

    UE_LOG(LogFortLoot, Log, TEXT("UFortWeaponModSetData::ApplyModSetToItemEntry applying mod set %s to %s, %d mods for rarity, %d slots"), *GetName(), *WeaponItemDefinition->GetName(), Mods->Num(), WeaponModSlots.Num());

    const FGameplayTag NameModifierTag = FGameplayTag::RequestGameplayTag(FName(TEXT("ItemStateValue.ConstObject.NameModifer")), false);

    if (NameModifierTag.IsValid())
        ItemEntry.SetStateConstObject(NameModifierTag, this);

    return true;
}

UClass* UFortWeaponPickupSpawnModData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponPickupSpawnModData"));

    return Class;
}

UDataTable* UFortWeaponPickupSpawnModData::GetModSetDataTable() const
{
    static auto ModSetDataTable = StaticClass()->FindPropertyByName(FName(TEXT("ModSetDataTable")));

    return *ModSetDataTable->ContainerPtrToValuePtr<UDataTable*>(this);
}

const UFortWeaponModSetData* UFortWeaponPickupSpawnModData::TryGetRandomModSetForWeaponItemDefinition(const UFortWeaponItemDefinition* WeaponItemDefinition) const
{
    auto ModSetDataTable = GetModSetDataTable();
    const FGameplayTag BucketTag = WeaponItemDefinition ? WeaponItemDefinition->GetWeaponModBucketTag() : FGameplayTag();

    if (!ModSetDataTable || !BucketTag.IsValid())
        return nullptr;

    TArray<const FFortWeaponModSetRow*> BucketEntries;
    float TotalWeight = 0.0f;

    for (auto& RowPair : ModSetDataTable->RowMap)
    {
        auto Row = (const FFortWeaponModSetRow*)RowPair.Value;

        if (Row && Row->GetSet() && Row->GetBucketTag() == BucketTag)
        {
            BucketEntries.Add(Row);
            TotalWeight += Row->GetRandomWeight();
        }
    }

    if (BucketEntries.Num() <= 0)
        return nullptr;

    if (BucketEntries.Num() == 1)
        return BucketEntries[0]->GetSet();

    float RandomWeight = FMath::FRand() * TotalWeight;

    for (auto BucketEntry : BucketEntries)
    {
        if (RandomWeight < BucketEntry->GetRandomWeight())
            return BucketEntry->GetSet();

        RandomWeight -= BucketEntry->GetRandomWeight();
    }

    return nullptr;
}

bool UFortWeaponPickupSpawnModData::TryApplyRandomModSetToItemEntry(FFortItemEntry& ItemEntry) const
{
    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry.GetItemDefinition());

    if (!WeaponItemDefinition)
        return false;

    auto ModSet = TryGetRandomModSetForWeaponItemDefinition(WeaponItemDefinition);

    if (!ModSet)
        return false;

    return UFortWeaponModFunctionLibrary::ApplyModSetToItemEntry(ModSet, ItemEntry);
}

UClass* UFortWeaponModManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponModManager"));

    return Class;
}

UFortWeaponModManager* UFortWeaponModManager::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    return World && StaticClass() ? (UFortWeaponModManager*)World->GetSubsystemBase(StaticClass()) : nullptr;
}

UFortWeaponPickupSpawnModData* UFortWeaponModManager::GetAutoApplyModDataAsset() const
{
    static auto AutoApplyModDataAsset = StaticClass()->FindPropertyByName(FName(TEXT("AutoApplyModDataAsset")));

    return AutoApplyModDataAsset ? *AutoApplyModDataAsset->ContainerPtrToValuePtr<UFortWeaponPickupSpawnModData*>(this) : nullptr;
}

bool UFortWeaponModFunctionLibrary::ApplyWeaponModToItemEntry(FFortItemEntry& ItemEntry, UFortWeaponModItemDefinition* WeaponMod)
{
    auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry.GetItemDefinition());

    if (!WeaponItemDefinition || !WeaponMod->CanBeUsedByWeaponDefinition(WeaponItemDefinition))
        return false;

    for (auto& WeaponModSlot : ItemEntry.GetWeaponModSlots())
    {
        if (WeaponModSlot.WeaponMod == WeaponMod)
            return false;
    }

    ItemEntry.AppendWeaponModSlot(FFortWeaponModSlot{ WeaponMod, true });

    return true;
}

bool UFortWeaponModFunctionLibrary::ApplyModSetToItemEntry(const UFortWeaponModSetData* ModSet, FFortItemEntry& ItemEntry)
{
    return ModSet && ModSet->ApplyModSetToItemEntry(ItemEntry);
}

bool UFortWeaponModFunctionLibrary::TryAddWeaponMod(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon)
{
    if (!WeaponMod || !Weapon)
        return false;

    if (!Weapon->AddWeaponModSlot(FFortWeaponModSlot{ WeaponMod, true }))
        return false;

    Weapon->UpdateCurrentWeaponModSlotsOnWorldItem();

    return true;
}

bool UFortWeaponModFunctionLibrary::TryRemoveWeaponMod(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon)
{
    if (!WeaponMod || !Weapon)
        return false;

    auto& WeaponModSlots = Weapon->GetWeaponModSlots();

    for (int32 SlotIndex = WeaponModSlots.Num() - 1; SlotIndex >= 0; SlotIndex--)
    {
        if (WeaponModSlots[SlotIndex].WeaponMod == WeaponMod)
        {
            if (!Weapon->RemoveWeaponModSlot(SlotIndex))
                return false;

            Weapon->UpdateCurrentWeaponModSlotsOnWorldItem();

            return true;
        }
    }

    return false;
}

void UFortWeaponModFunctionLibrary::AssignPlayerNameToItemEntry(AFortPlayerStateAthena* PlayerState, FFortItemEntry& ItemEntry)
{
    const FGameplayTag NameModifierTag = FGameplayTag::RequestGameplayTag(FName(TEXT("ItemStateValue.ConstObject.NameModifer")), false);

    if (PlayerState && NameModifierTag.IsValid())
        ItemEntry.SetStateConstObject(NameModifierTag, PlayerState);
}

void UFortWeaponModFunctionLibrary::AssignPlayerNameToWeapon(AFortPlayerStateAthena* PlayerState, AFortWeapon* Weapon)
{
    auto Item = Weapon ? Weapon->GetInventoryItem() : nullptr;

    if (Item)
        AssignPlayerNameToItemEntry(PlayerState, Item->GetItemEntry());
}

void UFortWeaponModFunctionLibrary::RemoveModSetNameFromItemEntry(FFortItemEntry& ItemEntry)
{
    const FGameplayTag NameModifierTag = FGameplayTag::RequestGameplayTag(FName(TEXT("ItemStateValue.ConstObject.NameModifer")), false);

    if (NameModifierTag.IsValid() && Cast<UFortWeaponModSetData>(ItemEntry.GetStateConstObject(NameModifierTag)))
        ItemEntry.RemoveStateConstObject(NameModifierTag);
}

void UFortWeaponModFunctionLibrary::RemoveModSetNameFromWeapon(AFortWeapon* Weapon)
{
    auto Item = Weapon ? Weapon->GetInventoryItem() : nullptr;

    if (Item)
        RemoveModSetNameFromItemEntry(Item->GetItemEntry());
}
