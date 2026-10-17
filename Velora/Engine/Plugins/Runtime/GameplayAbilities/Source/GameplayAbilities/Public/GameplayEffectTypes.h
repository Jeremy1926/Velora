#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"

struct FActiveGameplayEffectHandle
{
    FActiveGameplayEffectHandle()
        : Handle(INDEX_NONE)
        , bPassedFiltersAndWasExecuted(false)
    {
    }

private:
    int32 Handle;
    bool bPassedFiltersAndWasExecuted;
};
