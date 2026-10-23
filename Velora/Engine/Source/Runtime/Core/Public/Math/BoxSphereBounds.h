#pragma once

#include "Math/Vector.h"

struct FBoxSphereBounds
{
    FVector Origin;
    FVector BoxExtent;
    float SphereRadius;
};

namespace UE::Math
{
    template<typename T>
    struct TBoxSphereBounds
    {
        TVector<T> Origin;
        TVector<T> BoxExtent;
        T SphereRadius;

        TBoxSphereBounds() = default;

        FORCEINLINE explicit TBoxSphereBounds(const FBoxSphereBounds& Bounds)
            : Origin(Bounds.Origin)
            , BoxExtent(Bounds.BoxExtent)
            , SphereRadius(Bounds.SphereRadius)
        {
        }

        FORCEINLINE explicit operator FBoxSphereBounds() const
        {
            return FBoxSphereBounds{ (FVector)Origin, (FVector)BoxExtent, (float)SphereRadius };
        }
    };
}

using FBoxSphereBounds3d = UE::Math::TBoxSphereBounds<double>;

template<>
struct TGameLayout<FBoxSphereBounds> : TLWCGameLayout<FBoxSphereBounds, FBoxSphereBounds3d>
{
};
