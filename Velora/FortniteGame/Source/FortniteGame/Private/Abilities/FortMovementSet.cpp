#include "pch.h"
#include "Abilities/FortMovementSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

FFortGameplayAttributeData& UFortMovementSet::GetFlySpeed()
{
    static auto FlySpeed = GetClass()->FindPropertyByName(FName(TEXT("FlySpeed")));

    return *FlySpeed->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}
