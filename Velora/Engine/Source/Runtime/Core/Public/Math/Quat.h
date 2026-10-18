#pragma once

#include "HAL/Platform.h"
#include "Math/Rotator.h"

struct alignas(16) FQuat
{
    float X;
    float Y;
    float Z;
    float W;

    FORCEINLINE FQuat()
    {
    }

    FORCEINLINE FQuat(float InX, float InY, float InZ, float InW)
        : X(InX)
        , Y(InY)
        , Z(InZ)
        , W(InW)
    {
    }

    FORCEINLINE explicit FQuat(const FRotator& R)
    {
        *this = R.Quaternion();
    }

    FORCEINLINE bool operator!=(const FQuat& Q) const
    {
        return X != Q.X || Y != Q.Y || Z != Q.Z || W != Q.W;
    }

    FRotator Rotator() const;
};
