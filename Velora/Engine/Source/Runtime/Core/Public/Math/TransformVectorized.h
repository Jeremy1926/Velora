#pragma once

#include "HAL/Platform.h"
#include "Math/Vector.h"
#include <xmmintrin.h>

typedef __m128 VectorRegister;

struct alignas(16) FTransform
{
public:
    FORCEINLINE FVector GetTranslation() const
    {
        return FVector(Translation.m128_f32[0], Translation.m128_f32[1], Translation.m128_f32[2]);
    }

protected:
    VectorRegister Rotation;
    VectorRegister Translation;
    VectorRegister Scale3D;
};
