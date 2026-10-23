#pragma once

#include "HAL/Platform.h"
#include "Math/Rotator.h"

struct alignas(16) FQuat
{
    float X;
    float Y;
    float Z;
    float W;

    static const FQuat Identity;

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

namespace UE::Math
{
    template<typename T>
    struct alignas(16) TQuat
    {
        T X;
        T Y;
        T Z;
        T W;

        TQuat() = default;

        FORCEINLINE explicit TQuat(const FQuat& Q)
            : X(Q.X)
            , Y(Q.Y)
            , Z(Q.Z)
            , W(Q.W)
        {
        }

        FORCEINLINE explicit operator FQuat() const
        {
            return FQuat((float)X, (float)Y, (float)Z, (float)W);
        }
    };
}

using FQuat4d = UE::Math::TQuat<double>;

template<>
struct TGameLayout<FQuat> : TLWCGameLayout<FQuat, FQuat4d>
{
};

