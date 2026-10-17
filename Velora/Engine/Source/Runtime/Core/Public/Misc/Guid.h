#pragma once

#include "HAL/Platform.h"

struct FGuid
{
public:
    FGuid()
        : A(0)
        , B(0)
        , C(0)
        , D(0)
    {
    }

    friend bool operator==(const FGuid& X, const FGuid& Y)
    {
        return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) == 0;
    }

    friend bool operator!=(const FGuid& X, const FGuid& Y)
    {
        return ((X.A ^ Y.A) | (X.B ^ Y.B) | (X.C ^ Y.C) | (X.D ^ Y.D)) != 0;
    }

    static FGuid NewGuid();

    uint32 A;
    uint32 B;
    uint32 C;
    uint32 D;
};
