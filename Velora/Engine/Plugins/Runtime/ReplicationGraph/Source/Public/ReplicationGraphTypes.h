#pragma once

#include "Containers/Map.h"

class AActor;
struct FGlobalActorReplicationInfo;

struct FGlobalActorReplicationInfoMap
{
public:
    void ResetActorMap();

private:
    TMap<AActor*, FGlobalActorReplicationInfo*> ActorMap;
};
