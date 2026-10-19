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

const int32& FFortBaseWeaponStats::GetBaseLevel() const
{
    static auto BaseLevel = StaticStruct()->FindPropertyByName(FName(TEXT("BaseLevel")));

    return *BaseLevel->ContainerPtrToValuePtr<int32>(this);
}

const int32& FFortBaseWeaponStats::GetClipSize() const
{
    static auto ClipSize = StaticStruct()->FindPropertyByName(FName(TEXT("ClipSize")));

    return *ClipSize->ContainerPtrToValuePtr<int32>(this);
}

const float& FFortBaseWeaponStats::GetClipScale() const
{
    static auto ClipScale = StaticStruct()->FindPropertyByName(FName(TEXT("ClipScale")));

    return *ClipScale->ContainerPtrToValuePtr<float>(this);
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

    return (int32)((FMath::Max((float)Level - (float)WeaponStats->GetBaseLevel(), 0.0f) * WeaponStats->GetClipScale() + 1.0f) * (float)WeaponStats->GetClipSize());
}

const FDataTableRowHandle& UFortWeaponItemDefinition::GetWeaponStatHandle() const
{
    static auto WeaponStatHandle = GetClass()->FindPropertyByName(FName(TEXT("WeaponStatHandle")));

    return *WeaponStatHandle->ContainerPtrToValuePtr<FDataTableRowHandle>(this);
}
