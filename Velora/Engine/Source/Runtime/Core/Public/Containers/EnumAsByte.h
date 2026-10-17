#pragma once

#include "HAL/Platform.h"

template<class TEnum>
class TEnumAsByte
{
public:
    FORCEINLINE TEnumAsByte()
    {
    }

    FORCEINLINE TEnumAsByte(TEnum InValue)
        : Value(static_cast<uint8>(InValue))
    {
    }

private:
    uint8 Value;
};
