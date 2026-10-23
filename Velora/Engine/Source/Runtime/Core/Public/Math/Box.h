#pragma once

#include "Math/Vector.h"

struct FBox
{
    FORCEINLINE FVector GetClosestPointTo(const FVector& Point) const
    {
        FVector ClosestPoint = Point;

        if (Point.X < Min.X)
            ClosestPoint.X = Min.X;
        else if (Point.X > Max.X)
            ClosestPoint.X = Max.X;

        if (Point.Y < Min.Y)
            ClosestPoint.Y = Min.Y;
        else if (Point.Y > Max.Y)
            ClosestPoint.Y = Max.Y;

        if (Point.Z < Min.Z)
            ClosestPoint.Z = Min.Z;
        else if (Point.Z > Max.Z)
            ClosestPoint.Z = Max.Z;

        return ClosestPoint;
    }

    FVector Min;
    FVector Max;
    uint8 IsValid;
};

namespace UE::Math
{
    template<typename T>
    struct TBox
    {
        TVector<T> Min;
        TVector<T> Max;
        uint8 IsValid;

        TBox() = default;

        FORCEINLINE explicit TBox(const FBox& Box)
            : Min(Box.Min)
            , Max(Box.Max)
            , IsValid(Box.IsValid)
        {
        }

        FORCEINLINE explicit operator FBox() const
        {
            return FBox{ (FVector)Min, (FVector)Max, IsValid };
        }
    };
}

using FBox3d = UE::Math::TBox<double>;

template<>
struct TGameLayout<FBox> : TLWCGameLayout<FBox, FBox3d>
{
};
