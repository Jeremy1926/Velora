#pragma once

#include "GameFramework/CharacterMovementComponent.h"

class UFortMovementComp_Character : public UCharacterMovementComponent
{
public:
    float GetMaxSpeed() const;
};
