#pragma once

#include "HAL/Platform.h"
#include "Containers/Set.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UNetConnection;

struct FNetworkObjectInfo
{
    AActor* Actor;
    TWeakObjectPtr<AActor> WeakActor;
    double NextUpdateTime;
    double LastNetReplicateTime;
    float OptimalNetUpdateDelta;
    float LastNetUpdateTime;
    uint32 bPendingNetUpdate : 1;
    uint32 bForceRelevantNextUpdate : 1;
    TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;
    TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;
};

class FNetworkObjectList
{
public:
    typedef TSet<TSharedPtr<FNetworkObjectInfo>> FNetworkObjectSet;

    void Remove(AActor* const Actor);

    const FNetworkObjectSet& GetActiveObjects() const
    {
        return ActiveNetworkObjects;
    }

private:
    FNetworkObjectSet AllNetworkObjects;
    FNetworkObjectSet ActiveNetworkObjects;
    FNetworkObjectSet ObjectsDormantOnAllConnections;
    TMap<TWeakObjectPtr<UNetConnection>, int32> NumDormantObjectsPerConnection;
};
