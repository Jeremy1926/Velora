#pragma once

#include "HAL/Platform.h"

struct FWindowsPlatformTLS
{
    static FORCEINLINE uint32 GetCurrentThreadId()
    {
        return ::GetCurrentThreadId();
    }
};
