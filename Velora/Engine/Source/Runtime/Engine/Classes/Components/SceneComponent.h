#pragma once

#include "UObject/Object.h"
#include "Math/TransformVectorized.h"
#include "Math/BoxSphereBounds.h"
#include "UObject/NameTypes.h"

class USceneComponent : public UObject
{
public:
    const FTransform& GetComponentTransform() const;

    FRotator GetComponentRotation() const;

    const FBoxSphereBounds& GetBounds() const;

    FBoxSphereBounds CalcLocalBounds() const;

    void K2_SetWorldLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport);

    FVector GetSocketLocation(FName InSocketName) const;

    FORCEINLINE FVector GetComponentLocation() const
    {
        return GetComponentTransform().GetTranslation();
    }
};
