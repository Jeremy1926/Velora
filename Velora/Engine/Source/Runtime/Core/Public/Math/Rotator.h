#pragma once

#include "HAL/Platform.h"
#include "Math/Vector.h"

struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;

    FVector Vector() const;
};
