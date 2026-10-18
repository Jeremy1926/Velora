#include "pch.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

UScriptStruct* FFortBaseWeaponStats::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortBaseWeaponStats"));

    return Struct;
}

UClass* UFortWeaponItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponItemDefinition"));

    return Class;
}

int32 UFortWeaponItemDefinition::GetBaseClipSizeWithMods(const FFortBaseWeaponStats* WeaponStats, int32 Level) const
{
    if (!WeaponStats)
        return 0;

    return (int32)((FMath::Max((float)Level - (float)WeaponStats->BaseLevel, 0.0f) * WeaponStats->ClipScale + 1.0f) * (float)WeaponStats->ClipSize);
}

const FDataTableRowHandle& UFortWeaponItemDefinition::GetWeaponStatHandle() const
{
    static auto WeaponStatHandle = GetClass()->FindPropertyByName(FName(TEXT("WeaponStatHandle")));

    return *WeaponStatHandle->ContainerPtrToValuePtr<FDataTableRowHandle>(this);
}
