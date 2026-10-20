#pragma once

#include "HAL/Platform.h"

struct FTimespan
{
public:
    FTimespan()
        : Ticks(0)
    {
    }

private:
    int64 Ticks;
};
