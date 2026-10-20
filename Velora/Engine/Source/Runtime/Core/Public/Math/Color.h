#pragma once

#include "HAL/Platform.h"

struct FLinearColor
{
    float R;
    float G;
    float B;
    float A;

    FORCEINLINE FLinearColor()
    {
    }

    FORCEINLINE FLinearColor(float InR, float InG, float InB, float InA = 1.0f)
        : R(InR)
        , G(InG)
        , B(InB)
        , A(InA)
    {
    }
};
