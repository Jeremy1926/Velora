#pragma once

#include "UObject/Object.h"
#include "Math/TransformVectorized.h"

class USceneComponent : public UObject
{
public:
    const FTransform& GetComponentTransform() const;

    FORCEINLINE FVector GetComponentLocation() const
    {
        return GetComponentTransform().GetTranslation();
    }
};
