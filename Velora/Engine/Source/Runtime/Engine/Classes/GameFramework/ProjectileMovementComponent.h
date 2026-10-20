#pragma once

#include "Components/ActorComponent.h"

class UProjectileMovementComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void SetProjectileGravityScale(float InProjectileGravityScale);
};
