#include "pch.h"
#include "Abilities/FortMovementSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

FFortGameplayAttributeData& UFortMovementSet::GetFlySpeed()
{
    static auto FlySpeed = GetClass()->FindPropertyByName(FName(TEXT("FlySpeed")));

    return *FlySpeed->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}

FFortGameplayAttributeData& UFortMovementSet::GetSpeedMultiplier()
{
    static auto SpeedMultiplier = GetClass()->FindPropertyByName(FName(TEXT("SpeedMultiplier")));

    return *SpeedMultiplier->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}
