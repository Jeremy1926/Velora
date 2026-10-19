#pragma once

#include "HAL/Platform.h"

template<class TEnum>
class TEnumAsByte
{
public:
    FORCEINLINE TEnumAsByte()
    {
    }

    FORCEINLINE TEnumAsByte(const TEnumAsByte& InValue)
        : Value(InValue.Value)
    {
    }

    FORCEINLINE TEnumAsByte(TEnum InValue)
        : Value(static_cast<uint8>(InValue))
    {
    }

    FORCEINLINE operator TEnum() const
    {
        return (TEnum)Value;
    }

private:
    uint8 Value;
};
