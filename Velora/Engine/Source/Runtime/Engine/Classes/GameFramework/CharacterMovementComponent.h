#pragma once

#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"

class UCharacterMovementComponent : public UActorComponent
{
public:
    void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0);

    bool GetCheatFlying() const;

    void SetCheatFlying(bool bInCheatFlying);

    EMovementMode GetMovementMode() const;

    float GetMaxFlySpeed() const;

    void SetMaxFlySpeed(float InMaxFlySpeed);
};
