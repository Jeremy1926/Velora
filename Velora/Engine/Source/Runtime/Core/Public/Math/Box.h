#pragma once

#include "Math/Vector.h"

struct FBox
{
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
