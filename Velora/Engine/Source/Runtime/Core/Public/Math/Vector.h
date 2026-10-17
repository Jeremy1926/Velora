#pragma once

#include "HAL/Platform.h"

enum EForceInit
{
    ForceInit,
    ForceInitToZero
};

struct FVector
{
    float X;
    float Y;
    float Z;

    FORCEINLINE FVector()
    {
    }

    FORCEINLINE explicit FVector(EForceInit)
        : X(0.0f)
        , Y(0.0f)
        , Z(0.0f)
    {
    }

    FORCEINLINE FVector(float InX, float InY, float InZ)
        : X(InX)
        , Y(InY)
        , Z(InZ)
    {
    }

    FORCEINLINE FVector operator-(const FVector& V) const
    {
        return FVector(X - V.X, Y - V.Y, Z - V.Z);
    }

    FORCEINLINE float operator|(const FVector& V) const
    {
        return X * V.X + Y * V.Y + Z * V.Z;
    }

    FORCEINLINE float SizeSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }
};
