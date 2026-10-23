#pragma once

#include "Engine/ReplicationDriver.h"
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Templates/SharedPointer.h"
#include "ReplicationGraphTypes.h"

class AActor;
class UNetConnection;
class UNetDriver;
class UReplicationGraph;
class UWorld;

struct FReplicationGraphGlobalData
{
    FGlobalActorReplicationInfoMap* GlobalActorReplicationInfoMap;
};

class UReplicationGraphNode : public UObject
{
public:
    void NotifyResetAllNetworkActors();
};

class UNetReplicationGraphConnection : public UReplicationConnectionDriver
{
public:
    static UClass* StaticClass();

    UNetConnection*& GetNetConnection();

    int32& GetConnectionOrderNum();

    int32& GetConnectionId();

    FGlobalActorReplicationInfoMap*& GetGlobalMap();

    void NotifyResetAllNetworkActors();

private:
    friend UReplicationGraph;

    void InitForGraph(UReplicationGraph* Graph);

    void InitForConnection(UNetConnection* InConnection);
};

class UReplicationGraph : public UReplicationDriver
{
public:
    static UClass* StaticClass();

    UClass* GetReplicationConnectionManagerClass();

    UNetDriver*& GetNetDriver();

    TArray<UNetReplicationGraphConnection*>& GetConnections();

    TArray<UNetReplicationGraphConnection*>& GetPendingConnectionManagers();

    TArray<UReplicationGraphNode*>& GetGlobalGraphNodes();

    TSharedPtr<FReplicationGraphGlobalData>& GetGraphGlobals();

    FGlobalActorReplicationInfoMap& GetGlobalActorReplicationInfoMap();

    TSet<AActor*>& GetActiveNetworkActors();

    void InitForNetDriver(UNetDriver* InNetDriver);

    void InitializeActorsInWorld(UWorld* InWorld);

    void AddClientConnection(UNetConnection* NetConnection);

    void AddNetworkActor(AActor* Actor);

    void InitializeForWorld(UWorld* World);

    void InitGlobalActorClassSettings();

    void InitGlobalGraphNodes();

    void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager);

private:
    UNetReplicationGraphConnection* CreateClientConnectionManagerInternal(UNetConnection* Connection);
};
