#pragma once

#include "UObject/Object.h"

class AActor;

class UReplicationDriver : public UObject
{
public:
    void FlushNetDormancy(AActor* Actor, bool bWasDormInitial);

    int32 ServerReplicateActors(float DeltaSeconds);
};

class UReplicationConnectionDriver : public UObject
{
};
