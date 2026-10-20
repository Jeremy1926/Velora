#pragma once

#include "GameFramework/ProjectileMovementComponent.h"
#include "Math/Vector.h"

class UFortProjectileMovementComponent : public UProjectileMovementComponent
{
public:
    static UClass* StaticClass();

    void SetVelocity(const FVector& InReplicatedVelocity);
};
