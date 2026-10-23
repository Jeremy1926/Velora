#pragma once

#include "HAL/Platform.h"
#include "UObject/NameTypes.h"

class UChannel;

class FOutBunch
{
public:
    static int32 GetTypeSize();

    static void* operator new(size_t Size);

    static void operator delete(void* Ptr);

    FOutBunch();

    FOutBunch(UChannel* InChannel, bool bInClose);

    ~FOutBunch();

    int64 GetNumBits() const;

    void Reset();

    void SetNext(FOutBunch* InNext);

    void SetChannel(UChannel* InChannel);

    void SetChIndex(int32 InChIndex);

    void SetChName(const FName& InChName);

    void SetReliable(bool bInReliable);

    void SetDormant(bool bInDormant);
};
