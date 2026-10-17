#pragma once

#include "HAL/Platform.h"

class UChannel;

class FOutBunch
{
public:
    static void* operator new(size_t Size);

    static void operator delete(void* Ptr);

    FOutBunch(UChannel* InChannel, bool bInClose);

    ~FOutBunch();

    void SetReliable(bool bInReliable);

    void SetDormant(bool bInDormant);
};
