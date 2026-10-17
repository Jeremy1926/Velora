#pragma once

#include "HAL/Platform.h"

template<bool HAS_EDITOR_DATA, bool IS_DEDICATED_SERVER, bool IS_CLIENT_ONLY>
struct FWindowsPlatformProperties
{
    static FORCEINLINE const char* IniPlatformName()
    {
        return "Windows";
    }
};
