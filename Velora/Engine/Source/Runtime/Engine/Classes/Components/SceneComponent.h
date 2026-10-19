#pragma once

#include "UObject/Object.h"
#include "Math/TransformVectorized.h"
#include "Math/BoxSphereBounds.h"

class USceneComponent : public UObject
{
public:
    const FTransform& GetComponentTransform() const;

    FRotator GetComponentRotation() const;

    const FBoxSphereBounds& GetBounds() const;

    void K2_SetWorldLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport);

    FORCEINLINE FVector GetComponentLocation() const
    {
        return GetComponentTransform().GetTranslation();
    }
};
