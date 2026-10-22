#pragma once

#include "HAL/Platform.h"

template<typename OptionalType>
struct TOptional
{
    bool IsSet() const
    {
        return bIsSet;
    }

    const OptionalType& GetValue() const
    {
        return Value;
    }

    OptionalType Value{};
    bool bIsSet = false;
};
