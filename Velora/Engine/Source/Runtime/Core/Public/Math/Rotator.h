#pragma once

#include "HAL/Platform.h"
#include "Math/Vector.h"

struct FQuat;

struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;

    FVector Vector() const;

    FQuat Quaternion() const;

    FORCEINLINE bool Equals(const FRotator& R, float Tolerance = 1.e-4f) const
    {
        return FMath::Abs(NormalizeAxis(Pitch - R.Pitch)) <= Tolerance && FMath::Abs(NormalizeAxis(Yaw - R.Yaw)) <= Tolerance && FMath::Abs(NormalizeAxis(Roll - R.Roll)) <= Tolerance;
    }

    static FORCEINLINE float ClampAxis(float Angle)
    {
        Angle = fmodf(Angle, 360.f);

        if (Angle < 0.f)
            Angle += 360.f;

        return Angle;
    }

    static FORCEINLINE float NormalizeAxis(float Angle)
    {
        Angle = ClampAxis(Angle);

        if (Angle > 180.f)
            Angle -= 360.f;

        return Angle;
    }
};

namespace UE::Math
{
    template<typename T>
    struct TRotator
    {
        T Pitch;
        T Yaw;
        T Roll;

        TRotator() = default;

        FORCEINLINE explicit TRotator(const FRotator& R)
            : Pitch(R.Pitch)
            , Yaw(R.Yaw)
            , Roll(R.Roll)
        {
        }

        FORCEINLINE explicit operator FRotator() const
        {
            return FRotator{ (float)Pitch, (float)Yaw, (float)Roll };
        }
    };
}

using FRotator3d = UE::Math::TRotator<double>;

template<>
struct TGameLayout<FRotator> : TLWCGameLayout<FRotator, FRotator3d>
{
};

