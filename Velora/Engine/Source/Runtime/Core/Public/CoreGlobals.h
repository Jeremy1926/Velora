#pragma once

#include "HAL/Platform.h"
#include "HAL/PlatformTLS.h"

class FOutputDeviceRedirector;

extern bool& GIsClient;
extern bool& GIsServer;
extern uint32& GGameThreadId;
extern uint32* GSlateLoadingThreadId;
extern bool& GIsGameThreadIdInitialized;

FOutputDeviceRedirector* GetGlobalLogSingleton();

#define GLog GetGlobalLogSingleton()

FORCEINLINE bool IsInGameThread()
{
    if (GIsGameThreadIdInitialized)
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();

        return CurrentThreadId == GGameThreadId || (GSlateLoadingThreadId && CurrentThreadId == *GSlateLoadingThreadId);
    }

    return true;
}
