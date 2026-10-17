#pragma once

#include "HAL/Platform.h"

template<int32 Size, uint32 Alignment>
struct alignas(Alignment) TAlignedBytes
{
    uint8 Pad[Size];
};
