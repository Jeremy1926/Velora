#pragma once

#include "Templates/SharedPointer.h"

class FUniqueNetId;

struct FUniqueNetIdWrapper
{
    virtual ~FUniqueNetIdWrapper()
    {
    }

protected:
    TSharedPtr<const FUniqueNetId> UniqueNetId;
};
