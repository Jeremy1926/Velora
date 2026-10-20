#pragma once

#include "HAL/Platform.h"
#include "Math/UnrealMathUtility.h"

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

    FORCEINLINE FVector operator+(const FVector& V) const
    {
        return FVector(X + V.X, Y + V.Y, Z + V.Z);
    }

    FORCEINLINE FVector operator*(float Scale) const
    {
        return FVector(X * Scale, Y * Scale, Z * Scale);
    }

    FORCEINLINE FVector operator-(const FVector& V) const
    {
        return FVector(X - V.X, Y - V.Y, Z - V.Z);
    }

    FORCEINLINE float operator|(const FVector& V) const
    {
        return X * V.X + Y * V.Y + Z * V.Z;
    }

    FORCEINLINE float Size() const
    {
        return FMath::Sqrt(X * X + Y * Y + Z * Z);
    }

    FORCEINLINE float SizeSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }

    FORCEINLINE bool Normalize(float Tolerance = 1.e-8f)
    {
        const float SquareSum = X * X + Y * Y + Z * Z;

        if (SquareSum > Tolerance)
        {
            const float Scale = FMath::InvSqrt(SquareSum);
            X *= Scale;
            Y *= Scale;
            Z *= Scale;

            return true;
        }

        return false;
    }

    FORCEINLINE FVector GetSafeNormal(float Tolerance = 1.e-8f) const
    {
        const float SquareSum = X * X + Y * Y + Z * Z;

        if (SquareSum == 1.f)
            return *this;
        else if (SquareSum < Tolerance)
            return FVector(ForceInitToZero);

        const float Scale = FMath::InvSqrt(SquareSum);

        return FVector(X * Scale, Y * Scale, Z * Scale);
    }

    FORCEINLINE bool Equals(const FVector& V, float Tolerance = 1.e-4f) const
    {
        return FMath::Abs(X - V.X) <= Tolerance && FMath::Abs(Y - V.Y) <= Tolerance && FMath::Abs(Z - V.Z) <= Tolerance;
    }
};
