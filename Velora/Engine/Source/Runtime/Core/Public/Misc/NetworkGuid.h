#pragma once

#include "HAL/Platform.h"

class FNetworkGUID
{
public:
    uint32 Value;

    FNetworkGUID()
        : Value(0)
    {
    }

    friend bool operator==(const FNetworkGUID& X, const FNetworkGUID& Y)
    {
        return X.Value == Y.Value;
    }

    friend uint32 GetTypeHash(const FNetworkGUID& Guid)
    {
        return Guid.Value;
    }
};
