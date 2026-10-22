#pragma once

#include "HAL/Platform.h"

struct FCrc
{
    static uint32 Strihash_DEPRECATED(const TCHAR* Data);

    static uint32 MemCrc32(const void* Data, int32 Length, uint32 CRC = 0);
};
