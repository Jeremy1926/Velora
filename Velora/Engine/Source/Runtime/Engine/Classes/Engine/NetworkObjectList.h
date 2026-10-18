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

    TWeakObjectPtr<AActor> GetWeakActor() const;

    double& GetNextUpdateTime();

    double& GetLastNetReplicateTime();

    float& GetOptimalNetUpdateDelta();

    float& GetLastNetUpdateTime();

    bool GetPendingNetUpdate() const;

    void SetPendingNetUpdate(bool bInPendingNetUpdate);

    bool GetForceRelevantNextUpdate() const;

    void SetForceRelevantNextUpdate(bool bInForceRelevantNextUpdate);

    TSet<TWeakObjectPtr<UNetConnection>>& GetDormantConnections();

    TSet<TWeakObjectPtr<UNetConnection>>& GetRecentlyDormantConnections();

private:
    static int32 GetFlagsOffset();
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
