#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"

class UFortWeaponItemDefinition;
class UFortWeaponModItemDefinition;
struct FFortItemEntry;
enum class EFortRarity : uint8;

struct FFortWeaponModSlot
{
    UFortWeaponModItemDefinition* WeaponMod;
    bool bIsDynamic;
};

struct FFortSavedWeaponModSlot
{
    FFortSavedWeaponModSlot(const FFortWeaponModSlot& InSlot);

    FString WeaponModTemplateID;
    bool bIsDynamic;
};

struct FFortModSetModEntry
{
    UFortWeaponModItemDefinition* ModDefinition;
};

struct FFortWeaponModSetRow : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    FGameplayTag GetBucketTag() const;

    class UFortWeaponModSetData* GetSet() const;

    float GetRandomWeight() const;
};

class UFortWeaponModItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    FGameplayTag GetModSlot() const;

    bool CanBeUsedByWeaponDefinition(const UFortWeaponItemDefinition* WeaponDefinition) const;
};

class UFortWeaponModSetData : public UDataAsset
{
public:
    static UClass* StaticClass();

    const TArray<FFortModSetModEntry>* GetModsForRarity(EFortRarity Rarity) const;

    bool ApplyModSetToItemEntry(FFortItemEntry& ItemEntry) const;
};

class UFortWeaponPickupSpawnModData : public UDataAsset
{
public:
    static UClass* StaticClass();

    UDataTable* GetModSetDataTable() const;

    const UFortWeaponModSetData* TryGetRandomModSetForWeaponItemDefinition(const UFortWeaponItemDefinition* WeaponItemDefinition) const;

    bool TryApplyRandomModSetToItemEntry(FFortItemEntry& ItemEntry) const;
};

class UFortWeaponModManager : public UObject
{
public:
    static UClass* StaticClass();

    static UFortWeaponModManager* Get(const UObject* WorldContextObject);

    UFortWeaponPickupSpawnModData* GetAutoApplyModDataAsset() const;
};

class UFortWeaponModFunctionLibrary : public UObject
{
public:
    static bool ApplyWeaponModToItemEntry(FFortItemEntry& ItemEntry, UFortWeaponModItemDefinition* WeaponMod);

    static bool ApplyModSetToItemEntry(const UFortWeaponModSetData* ModSet, FFortItemEntry& ItemEntry);
};
