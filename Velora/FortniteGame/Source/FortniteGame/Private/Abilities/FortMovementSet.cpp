#include "pch.h"
#include "Abilities/FortMovementSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortMovementSet::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMovementSet"));

    return Class;
}

FFortGameplayAttributeData* UFortMovementSet::GetFlySpeed()
{
    static auto FlySpeed = GetClass()->FindPropertyByName(FName(TEXT("FlySpeed")));

    return FlySpeed ? FlySpeed->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this) : nullptr;
}

FFortGameplayAttributeData& UFortMovementSet::GetSpeedMultiplier()
{
    static auto SpeedMultiplier = GetClass()->FindPropertyByName(FName(TEXT("SpeedMultiplier")));

    return *SpeedMultiplier->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}
