#pragma once

#include "HAL/Platform.h"

struct FActiveGameplayEffectHandle
{
private:
    int32 Handle;
    bool bPassedFiltersAndWasExecuted;
};
