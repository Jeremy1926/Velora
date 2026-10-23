#pragma once

#include "Engine/ReplicationDriver.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Delegates/Delegate.h"
#include "Templates/SharedPointer.h"
#include "UObject/ObjectKey.h"
#include "UObject/UObjectGlobals.h"
#include "ReplicationGraphTypes.h"

class AActor;
class UActorChannel;
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

    TArray<UReplicationGraphNode*>& GetAllChildNodes();

    TSharedPtr<FReplicationGraphGlobalData, ESPMode::ThreadSafe>& GetGraphGlobals();

    template<class T>
    T* CreateChildNode()
    {
        auto NewNode = NewObject<T>(this, T::StaticClass());
        NewNode->GetGraphGlobals() = GetGraphGlobals();
        GetAllChildNodes().Add(NewNode);

        return NewNode;
    }
};

class UReplicationGraphNode_ActorList : public UReplicationGraphNode
{
public:
    FActorRepListRefView& GetReplicationActorList();

    FStreamingLevelActorListCollection& GetStreamingLevelCollection();
};

class UReplicationGraphNode_ConnectionDormancyNode : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    FObjectKey& GetConnectionOwner();

    uint32& GetLastGatheredFrame();

    int32& GetTrickleStartCounter();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void OnClientVisibleLevelNameAdd(FName LevelName, UWorld* World);

private:
    void ConditionalGatherDormantActorsForConnection(FActorRepListRefView& ConnectionList, const FConnectionGatherActorListParameters& Params, FActorRepListRefView* RemovedList);

    FStreamingLevelActorListCollection& GetRemovedStreamingLevelActorListCollection();
};

class UReplicationGraphNode_DormancyNode : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    UReplicationGraphNode_ConnectionDormancyNode* GetExistingConnectionNode(const FConnectionGatherActorListParameters& Params);

private:
    UReplicationGraphNode_ConnectionDormancyNode* CreateConnectionNode(const FConnectionGatherActorListParameters& Params);

    TArray<TPair<FObjectKey, UReplicationGraphNode_ConnectionDormancyNode*>>& GetConnectionNodes();
};

class UReplicationGraphNode_ActorListFrequencyBuckets : public UReplicationGraphNode
{
public:
    struct FSettings
    {
        int32 NumBuckets;
        int32 ListSize;
        bool EnableFastPath;
        int32 FastPathFrameModulo;
    };

    static UClass* StaticClass();

    const FSettings& GetSettings();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    TArray<FActorRepListRefView, TInlineAllocator<2>>& GetNonStreamingCollection();

    FStreamingLevelActorListCollection& GetStreamingLevelCollection();
};

class UReplicationGraphNode_TearOff_ForConnection : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    struct FTearOffActorInfo
    {
        uint32 TearOffFrameNum;
        AActor* Actor;
        bool bHasReppedOnce;
    };

    TArray<FTearOffActorInfo>& GetTearOffActors();

    FActorRepListRefView& GetReplicationActorList();
};

class UNetReplicationGraphConnection : public UReplicationConnectionDriver
{
public:
    static UClass* StaticClass();

    UNetConnection*& GetNetConnection();

    int32& GetConnectionOrderNum();

    int32& GetConnectionId();

    FGlobalActorReplicationInfoMap*& GetGlobalMap();

    FPerConnectionActorInfoMap& GetActorInfoMap();

    TArray<FCachedDestructInfo>& GetPendingDestructInfoList();

    TSet<FActorDestructionInfo*>& GetTrackedDestructionInfoPtrs();

    TArray<FCachedDormantDestructInfo>& GetPendingDormantDestructList();

    TSet<FNetworkGUID>& GetTrackedDormantDestructionInfos();

    TMap<FName, TMulticastDelegate<void, FName, UWorld*>>& GetOnClientVisibleLevelNameAddMap();

    void NotifyActorChannelAdded(AActor* Actor, UActorChannel* Channel);

    void NotifyAddDestructionInfo(FActorDestructionInfo* DestructInfo);

    void NotifyAddDormantDestructionInfo(AActor* Actor);

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
