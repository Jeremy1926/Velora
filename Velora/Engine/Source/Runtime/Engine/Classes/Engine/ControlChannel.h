#pragma once

#include "Engine/Channel.h"

struct FActorDestructionInfo;

class UControlChannel : public UChannel
{
public:
    static UClass* StaticClass();

    int64 SendDestructionInfo(FActorDestructionInfo* DestructionInfo);
};
