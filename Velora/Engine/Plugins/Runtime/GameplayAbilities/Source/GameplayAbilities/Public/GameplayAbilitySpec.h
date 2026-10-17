#pragma once

#include "HAL/Platform.h"

struct FGameplayAbilitySpecHandle
{
    FGameplayAbilitySpecHandle()
        : Handle(-1)
    {
    }

private:
    int32 Handle;
};
