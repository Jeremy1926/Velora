#pragma once

#include "Containers/Array.h"
#include "UObject/CoreOnline.h"

struct FUniqueNetIdRepl : public FUniqueNetIdWrapper
{
    virtual ~FUniqueNetIdRepl()
    {
    }

    FUniqueNetIdRepl& operator=(const FUniqueNetIdRepl& Other);

protected:
    TArray<uint8> ReplicationBytes;
};
