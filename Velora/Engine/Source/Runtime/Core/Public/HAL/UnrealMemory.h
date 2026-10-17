#pragma once

#include "HAL/Platform.h"

struct FMemory
{
    static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 0);

    static void Free(void* Original);

    static SIZE_T QuantizeSize(SIZE_T Count, uint32 Alignment = 0);

    static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
    {
        return memset(Dest, 0, Count);
    }
};
