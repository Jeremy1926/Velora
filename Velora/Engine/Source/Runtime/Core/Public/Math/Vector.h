#pragma once

#include "HAL/Platform.h"
#include "Math/UnrealMathUtility.h"

struct FRotator;

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

    FRotator ToOrientationRotator() const;

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

    FORCEINLINE FVector RotateAngleAxis(const float AngleDeg, const FVector& Axis) const
    {
        const float S = sinf(AngleDeg * 0.017453292f);
        const float C = cosf(AngleDeg * 0.017453292f);

        const float XX = Axis.X * Axis.X;
        const float YY = Axis.Y * Axis.Y;
        const float ZZ = Axis.Z * Axis.Z;

        const float XY = Axis.X * Axis.Y;
        const float YZ = Axis.Y * Axis.Z;
        const float ZX = Axis.Z * Axis.X;

        const float XS = Axis.X * S;
        const float YS = Axis.Y * S;
        const float ZS = Axis.Z * S;

        const float OMC = 1.f - C;

        return FVector(
            (OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
            (OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z,
            (OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z
        );
    }

    FORCEINLINE bool Equals(const FVector& V, float Tolerance = 1.e-4f) const
    {
        return FMath::Abs(X - V.X) <= Tolerance && FMath::Abs(Y - V.Y) <= Tolerance && FMath::Abs(Z - V.Z) <= Tolerance;
    }
};

namespace UE::Math
{
    template<typename T>
    struct TVector
    {
        T X;
        T Y;
        T Z;

        TVector() = default;

        FORCEINLINE explicit TVector(const FVector& V)
            : X(V.X)
            , Y(V.Y)
            , Z(V.Z)
        {
        }

        FORCEINLINE explicit operator FVector() const
        {
            return FVector((float)X, (float)Y, (float)Z);
        }
    };
}

using FVector3d = UE::Math::TVector<double>;

template<>
struct TGameLayout<FVector> : TLWCGameLayout<FVector, FVector3d>
{
};

