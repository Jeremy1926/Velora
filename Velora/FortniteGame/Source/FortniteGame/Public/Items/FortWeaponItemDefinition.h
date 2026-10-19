#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "Engine/DataTable.h"

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

    int32 GetBaseClipSizeWithMods(const FFortBaseWeaponStats* WeaponStats, int32 Level) const;

    const FDataTableRowHandle& GetWeaponStatHandle() const;
};
