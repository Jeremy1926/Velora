#pragma once

#include "Math/Vector.h"

struct FBox
{
    FVector Min;
    FVector Max;
    uint8 IsValid;
};
