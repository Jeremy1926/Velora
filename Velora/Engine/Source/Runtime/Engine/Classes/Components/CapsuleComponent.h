#pragma once

#include "Components/PrimitiveComponent.h"

class UCapsuleComponent : public UPrimitiveComponent
{
public:
    float GetUnscaledCapsuleHalfHeight() const;

    float GetUnscaledCapsuleRadius() const;
};
