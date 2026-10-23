#pragma once

#include "Engine/ReplicationDriver.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Delegates/Delegate.h"
#include "Math/Box.h"
#include "Math/IntVector.h"
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
    UWorld* World;
};

class UReplicationGraphNode : public UObject
{
public:
    void NotifyResetAllNetworkActors();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo);

    void TearDown();

    void CleanChildNodes();

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

    void NotifyActorDormancyFlush(AActor* Actor);

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

    void ConditionalGatherDormantDynamicActors(FActorRepListRefView& RepList, const FConnectionGatherActorListParameters& Params, FActorRepListRefView* RemovedList, bool bEnforceReplistUniqueness = false, FActorRepListRefView* RemoveFromList = nullptr);

private:
    UReplicationGraphNode_ConnectionDormancyNode* CreateConnectionNode(const FConnectionGatherActorListParameters& Params);

    TArray<TPair<FObjectKey, UReplicationGraphNode_ConnectionDormancyNode*>>& GetConnectionNodes();
};

class UReplicationGraphNode_GridCell : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    UReplicationGraphNode_DormancyNode* GetDormancyNode();

    UReplicationGraphNode* GetDynamicNode();

    void AddDynamicActor(const FNewReplicatedActorInfo& ActorInfo);

    void RemoveDynamicActor(const FNewReplicatedActorInfo& ActorInfo);
};

class UReplicationGraphNode_GridSpatialization2D : public UReplicationGraphNode
{
public:
    struct FActorCellInfo
    {
        bool IsValid() const
        {
            return StartX != -1;
        }

        int32 StartX;
        int32 StartY;
        int32 EndX;
        int32 EndY;
    };

    struct FCachedDynamicActorInfo
    {
        FNewReplicatedActorInfo ActorInfo;
        FActorCellInfo CellInfo;
    };

    struct FCachedStaticActorInfo
    {
        FNewReplicatedActorInfo ActorInfo;
        bool bDormancyDriven;
    };

    struct FPendingStaticActors
    {
        AActor* Actor;
        bool DormancyDriven;
    };

    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void PrepareForReplication();

    void AddActorInternal_Static_Implementation(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven);

    void PutStaticActorIntoCell(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven);

    void HandleActorOutOfSpatialBounds(AActor* Actor, const FVector& Location3D, const bool bStaticActor);

    FActorCellInfo GetCellInfoForActor(AActor* Actor, const FVector& Location3D, float CullDistance);

    void GetGridNodesForActor(AActor* Actor, const FActorCellInfo& CellInfo, TArray<UReplicationGraphNode_GridCell*>& OutNodes);

private:
    struct FPlayerGridCellInformation
    {
        FIntPoint CurLocation;
        FIntPoint PrevLocation;
    };

    float& GetCellSize();

    uint8* GetSpatialBias();

    float& GetConnectionMaxZ();

    uint8* GetCreateCellNodeOverride();

    bool& GetDestroyDormantDynamicActors();

    TMap<AActor*, FCachedDynamicActorInfo>& GetDynamicSpatializedActors();

    TMap<AActor*, FCachedStaticActorInfo>& GetStaticSpatializedActors();

    TArray<FPendingStaticActors>& GetPendingStaticSpatializedActors();

    bool& GetNeedsRebuild();

    bool WillActorLocationGrowSpatialBounds(const FVector& Location);

    UReplicationGraphNode_GridCell* GetCellNode(UReplicationGraphNode_GridCell*& NodePtr);

    uint8* GetGridBounds();

    TArray<TArray<UReplicationGraphNode_GridCell*>>& GetGrid();

    TArray<UReplicationGraphNode_GridCell*>& GetGatheredNodes();

    TArray<UReplicationGraphNode_GridCell*>& GetGridX(int32 X)
    {
        auto& Grid = GetGrid();

        if (Grid.Num() <= X)
            Grid.SetNum(X + 1);

        return Grid[X];
    }

    UReplicationGraphNode_GridCell*& GetCell(TArray<UReplicationGraphNode_GridCell*>& GridX, int32 Y)
    {
        if (GridX.Num() <= Y)
            GridX.SetNum(Y + 1);

        return GridX[Y];
    }
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

    TArray<FLastLocationGatherInfo>& GetLastGatherLocations();

    FActorRepListRefView& GetPrevDormantActorListForNode(const UReplicationGraphNode* GridNode);

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
