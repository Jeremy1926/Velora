#pragma once

#include "HAL/Platform.h"

class FEngineVersionBase
{
public:
    FORCEINLINE uint16 GetMajor() const
    {
        return Major;
    }

    FORCEINLINE uint16 GetMinor() const
    {
        return Minor;
    }

    FORCEINLINE uint16 GetPatch() const
    {
        return Patch;
    }

    FORCEINLINE uint32 GetChangelist() const
    {
        return Changelist & (uint32)0x7fffffffU;
    }

protected:
    uint16 Major;
    uint16 Minor;
    uint16 Patch;
    uint32 Changelist;
};
