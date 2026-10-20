#pragma once

#include "HAL/Platform.h"

struct FMemory
{
    static void* Realloc(void* Original, SIZE_T Count, uint32 Alignment = 0);

    static void Free(void* Original);

    static SIZE_T QuantizeSize(SIZE_T Count, uint32 Alignment = 0);

    static FORCEINLINE void* Memset(void* Dest, uint8 Char, SIZE_T Count)
    {
        return memset(Dest, Char, Count);
    }

    static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
    {
        return memset(Dest, 0, Count);
    }

    static FORCEINLINE void Memswap(void* Ptr1, void* Ptr2, SIZE_T Size)
    {
        std::swap_ranges((uint8*)Ptr1, (uint8*)Ptr1 + Size, (uint8*)Ptr2);
    }
};
