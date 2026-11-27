#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

struct FFortItemEntry;
struct FFortWeaponModSlot;

struct FFortBaseWeaponStats : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    const int32& GetBaseLevel() const;

    const int32& GetClipSize() const;

    const float& GetClipScale() const;
};

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    template <class T>
    T* GetWeaponStats() const
    {
        return GetWeaponStatHandle().GetRow<T>(TEXT("UFortWeaponItemDefinition::GetWeaponStats"));
    }

    int32 GetBaseClipSizeWithMods(const UObject* WorldContextObject, const FFortItemEntry* ItemEntry, const FFortBaseWeaponStats* WeaponStats, int32 Level) const;

    void GetInitialAmmunitionQuantities(UObject* WorldContextObject, const FFortBaseWeaponStats* WeaponStats, const FFortItemEntry* ItemEntry, int32 Level, int32 AmmoOverride, int32& OutMagazineAmmoQuantity, int32& OutExtraAmmoQuantity) const;

    bool UsesPhantomReserveAmmo() const;

    const FDataTableRowHandle& GetWeaponStatHandle() const;

    const TArray<FFortWeaponModSlot>& GetWeaponModSlots() const;

    bool SetDefaultWeaponModSlotsIfNeeded(FFortItemEntry& ItemEntry) const;

    FGameplayTag GetWeaponModBucketTag() const;
};
