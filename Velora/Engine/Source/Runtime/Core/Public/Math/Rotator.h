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
