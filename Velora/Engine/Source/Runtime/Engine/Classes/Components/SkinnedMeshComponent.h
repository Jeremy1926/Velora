#pragma once

#include "Components/PrimitiveComponent.h"

class USkinnedMeshComponent : public UPrimitiveComponent
{
public:
    int32 GetBoneIndex(FName BoneName) const;
};
