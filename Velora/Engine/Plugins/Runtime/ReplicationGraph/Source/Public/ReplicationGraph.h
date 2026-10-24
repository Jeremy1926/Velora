#pragma once

#include "Engine/ReplicationDriver.h"
#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Delegates/Delegate.h"
#include "Math/Box.h"
#include "Math/IntVector.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"
#include "UObject/ObjectKey.h"
#include "UObject/UObjectGlobals.h"
#include "ReplicationGraphTypes.h"

class AActor;
class FOutBunch;
class UActorChannel;
class UFunction;
class UNetConnection;
class UNetDriver;
class UReplicationGraph;
class UWorld;
struct FFrame;
struct FOutParmRec;

struct FReplicationGraphGlobalData
{
    FGlobalActorReplicationInfoMap* GlobalActorReplicationInfoMap;
    UWorld* World;
    UReplicationGraph* ReplicationGraph;
};

class UReplicationGraphNode : public UObject
{
public:
    void NotifyResetAllNetworkActors();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void PrepareForReplication();

    void NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo);

    bool NotifyRemoveNetworkActor(const FNewReplicatedActorInfo& ActorInfo, bool bWarnIfNotFound = true);

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
    static UClass* StaticClass();

    FActorRepListRefView& GetReplicationActorList();

    FStreamingLevelActorListCollection& GetStreamingLevelCollection();
};

class UReplicationGraphNode_ConnectionDormancyNode : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    static void SetNumFramesUntilObsolete(uint32 NumFrames);

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

    static float& GetMaxZForConnection();

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

    TFunction<UReplicationGraphNode*(UReplicationGraphNode_GridCell*)>& GetCreateDynamicNodeOverride();

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

    void AddActor_Static(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo);

    void NotifyActorCullDistChange(AActor* Actor, FGlobalActorReplicationInfo& GlobalInfo, float OldDist);

    void AddActor_Dynamic(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo);

    void AddActor_Dormancy(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo);

    void RemoveActor_Static(const FNewReplicatedActorInfo& ActorInfo);

    void RemoveActor_Dynamic(const FNewReplicatedActorInfo& ActorInfo);

    void RemoveActor_Dormancy(const FNewReplicatedActorInfo& ActorInfo);

    void AddActorInternal_Static_Implementation(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven);

    void PutStaticActorIntoCell(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven);

    void HandleActorOutOfSpatialBounds(AActor* Actor, const FVector& Location3D, const bool bStaticActor);

    FActorCellInfo GetCellInfoForActor(AActor* Actor, const FVector& Location3D, float CullDistance);

    void GetGridNodesForActor(AActor* Actor, const FActorCellInfo& CellInfo, TArray<UReplicationGraphNode_GridCell*>& OutNodes);

    TFunction<UReplicationGraphNode_GridCell*(UReplicationGraphNode_GridSpatialization2D*)>& GetCreateCellNodeOverride();

    float& GetCellSize();

    uint8* GetSpatialBias();

    void AddToClassRebuildDenyList(UClass* Class);

protected:
    void AddActorInternal_Dynamic(const FNewReplicatedActorInfo& ActorInfo);

    void AddActorInternal_Static(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven);

    void RemoveActorInternal_Dynamic(const FNewReplicatedActorInfo& ActorInfo);

    void RemoveActorInternal_Static(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bWasAddedAsDormantActor);

private:
    struct FPlayerGridCellInformation
    {
        FIntPoint CurLocation;
        FIntPoint PrevLocation;
    };

    TClassMap<bool>& GetClassRebuildDenyList();

    float& GetConnectionMaxZ();

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
        struct FBucketThresholds
        {
            int32 MaxActors;
            int32 NumBuckets;
        };

        int32 NumBuckets;
        int32 ListSize;
        bool EnableFastPath;
        int32 FastPathFrameModulo;
        TArray<FBucketThresholds, TInlineAllocator<4>> BucketThresholds;
    };

    static UClass* StaticClass();

    static FSettings& GetDefaultSettings();

    const FSettings& GetSettings();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    TArray<FActorRepListRefView, TInlineAllocator<2>>& GetNonStreamingCollection();

    FStreamingLevelActorListCollection& GetStreamingLevelCollection();
};

class UReplicationGraphNode_DynamicSpatialFrequency : public UReplicationGraphNode_ActorList
{
public:
    struct FSpatializationZone
    {
        FSpatializationZone(float InMinDotProduct, float InMinDistPct, float InMaxDistPct, float InMinRepHz, float InMaxRepHz, float InMinRepHz_FastPath, float InMaxRepHz_FastPath, float TickRate)
            : MinDotProduct(InMinDotProduct)
            , MinDistPct(InMinDistPct)
            , MaxDistPct(InMaxDistPct)
            , MinRepPeriod(HzToFrm(InMinRepHz, TickRate))
            , MaxRepPeriod(HzToFrm(InMaxRepHz, TickRate))
            , FastPath_MinRepPeriod(HzToFrm(InMinRepHz_FastPath, TickRate))
            , FastPath_MaxRepPeriod(HzToFrm(InMaxRepHz_FastPath, TickRate))
        {
        }

        static uint32 HzToFrm(float Hz, float TargetFrameRate)
        {
            return Hz > 0.f ? (uint32)FMath::CeilToInt(TargetFrameRate / Hz) : 0;
        }

        float MinDotProduct;
        float MinDistPct;
        float MaxDistPct;
        uint32 MinRepPeriod;
        uint32 MaxRepPeriod;
        uint32 FastPath_MinRepPeriod;
        uint32 FastPath_MaxRepPeriod;
    };

    struct FSettings
    {
        TArrayView<FSpatializationZone> ZoneSettings;
        TArrayView<FSpatializationZone> ZoneSettings_NonFastSharedActors;
        int64 MaxBitsPerFrame;
        int32 MaxNearestActors;
    };

    static UClass* StaticClass();

    static FSettings& GetDefaultSettings();

    FSettings& GetSettings();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

protected:
    struct FDynamicSpatialFrequency_SortedItem
    {
        bool operator<(const FDynamicSpatialFrequency_SortedItem& Other) const
        {
            return FramesTillReplicate < Other.FramesTillReplicate;
        }

        AActor* Actor;
        int32 FramesTillReplicate;
        bool EnableFastPath;
        FGlobalActorReplicationInfo* GlobalInfo;
        FConnectionReplicationActorInfo* ConnectionInfo;
    };

    TArray<FDynamicSpatialFrequency_SortedItem>& GetSortedReplicationList();

    int32& GetNumExpectedReplicationsThisFrame();

    int32& GetNumExpectedReplicationsNextFrame();

    bool& GetIgnoreCullDistance();

    void GatherActors(const FActorRepListRefView& RepList, FGlobalActorReplicationInfoMap& GlobalMap, FPerConnectionActorInfoMap& ConnectionMap, const FConnectionGatherActorListParameters& Params, UNetConnection* NetConnection);

    void CalcFrequencyForActor(AActor* Actor, UReplicationGraph* RepGraph, UNetConnection* NetConnection, FGlobalActorReplicationInfo& GlobalInfo, FPerConnectionActorInfoMap& ConnectionMap, FSettings& MySettings, const FNetViewerArray& Viewers, const uint32 FrameNum, int32 ExistingItemIndex);
};

struct FAlwaysRelevantActorInfo
{
    bool operator==(const UNetConnection* InConnection) const
    {
        return Connection == InConnection;
    }

    UNetConnection* Connection;
    AActor* LastViewer;
    AActor* LastViewTarget;
};

class UReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    FActorRepListRefView& GetReplicationActorList();

    TArray<FAlwaysRelevantActorInfo>& GetPastRelevantActors();
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

    TMulticastDelegate<void, FName, UWorld*>& GetOnClientVisibleLevelNameAdd();

    TMap<FName, TMulticastDelegate<void, FName, UWorld*>>& GetOnClientVisibleLevelNameAddMap();

    TMulticastDelegate<void, FName>& GetOnClientVisibleLevelNameRemove();

    TArray<FLastLocationGatherInfo>& GetLastGatherLocations();

    TMulticastDelegate<void, UNetReplicationGraphConnection*, FPrioritizedRepList*>& GetOnPostReplicatePrioritizeLists();

    int32& GetQueuedBitsForActorDiscovery();

    TArray<FCachedDestructInfo>& GetOutOfRangeDestroyedActors();

    TArray<UReplicationGraphNode*>& GetConnectionGraphNodes();

    bool PrepareForReplication();

    void SetActorNotDormantOnConnection(AActor* InActor);

    void GetClientVisibleLevelNames(TSet<FName>& OutLevelNames);

    void UpdateGatherLocationsForConnection(const FNetViewerArray& ConnectionViewers, const FReplicationGraphDestructionSettings& DestructionSettings);

    void OnUpdateViewerLocation(FLastLocationGatherInfo* LocationInfo, const FNetViewer& Viewer, const FReplicationGraphDestructionSettings& DestructionSettings);

    int64 ReplicateDestructionInfos(const FNetViewerArray& Viewers, const float DestructInfoMaxDistanceSquared);

    int64 ReplicateDormantDestructionInfos();

    FActorRepListRefView& GetPrevDormantActorListForNode(const UReplicationGraphNode* GridNode);

    void NotifyActorChannelAdded(AActor* Actor, UActorChannel* Channel);

    void NotifyAddDestructionInfo(FActorDestructionInfo* DestructInfo);

    void NotifyAddDormantDestructionInfo(AActor* Actor);

    void NotifyResetAllNetworkActors();

private:
    friend UReplicationGraph;

    void InitForGraph(UReplicationGraph* Graph);

    void AddConnectionGraphNode(UReplicationGraphNode* Node);

    void RemoveConnectionGraphNode(UReplicationGraphNode* Node);

    void InitForConnection(UNetConnection* InConnection);
};

class UReplicationGraph : public UReplicationDriver
{
public:
    struct FPrioritizationConstants
    {
        float MaxDistanceScaling;
        uint32 MaxFramesSinceLastRep;
    };

    struct FFastSharedPathConstants
    {
        float DistanceRequirementPct;
        int32 MaxBitsPerFrame;
        int32 ListSkipPerFrame;
    };

    static UClass* StaticClass();

    UClass*& GetReplicationConnectionManagerClass();

    UNetDriver*& GetNetDriver();

    TArray<UNetReplicationGraphConnection*>& GetConnections();

    TArray<UNetReplicationGraphConnection*>& GetPendingConnectionManagers();

    TArray<UReplicationGraphNode*>& GetGlobalGraphNodes();

    TArray<UReplicationGraphNode*>& GetPrepareForReplicationNodes();

    TSharedPtr<FReplicationGraphGlobalData>& GetGraphGlobals();

    FGlobalActorReplicationInfoMap& GetGlobalActorReplicationInfoMap();

    TSet<AActor*>& GetActiveNetworkActors();

    TMap<FObjectKey, FRPCSendPolicyInfo>& GetRPCSendPolicyMap();

    TClassMap<bool>& GetRPC_Multicast_OpenChannelForClass();

    void InitForNetDriver(UNetDriver* InNetDriver);

    void InitializeActorsInWorld(UWorld* InWorld);

    void AddClientConnection(UNetConnection* NetConnection);

    UNetReplicationGraphConnection* FindOrAddConnectionManager(UNetConnection* NetConnection);

    void AddNetworkActor(AActor* Actor);

    void AddGlobalGraphNode(UReplicationGraphNode* GraphNode);

    void RemoveGlobalGraphNode(UReplicationGraphNode* GraphNode);

    void InitializeForWorld(UWorld* World);

    void InitGlobalActorClassSettings();

    void InitGlobalGraphNodes();

    void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager);

    void InitNode(UReplicationGraphNode* Node);

    template<class T>
    T* CreateNewNode()
    {
        auto NewNode = NewObject<T>(this, T::StaticClass());
        InitNode(NewNode);

        return NewNode;
    }

    float& GetDestructInfoMaxDistanceSquared();

    FPrioritizationConstants& GetPrioritizationConstants();

    FFastSharedPathConstants& GetFastSharedPathConstants();

    uint32& GetGlobalActorChannelFrameNumTimeout();

    FPrioritizedRepList& GetPrioritizedReplicationList();

    int32 ServerReplicateActors(float DeltaSeconds);

    void AddReplayViewers(UNetConnection* NetConnection, FNetViewerArray& Viewers);

    void PostServerReplicateStats(const FFrameReplicationStats& Stats);

    FFrameReplicationStats& GetFrameReplicationStats();

    bool& GetWasConnectionSaturated();

    uint32& GetReplicationGraphFrame();

    int32& GetActorDiscoveryMaxBitsPerFrame();

    void SetActorDiscoveryBudget(int32 ActorDiscoveryBudgetInKBytesPerSec);

    uint16 GetReplicationPeriodFrameForFrequency(float NetUpdateFrequency);

    TArray<UNetConnection*>& GetConnectionsNeedingsPostTickDispatchFlush();

    FOutBunch*& GetFastSharedReplicationBunch();

    UActorChannel*& GetFastSharedReplicationChannel();

    FName GetFastSharedReplicationFuncName();

    void SetFastSharedReplicationFuncName(const FName& InFastSharedReplicationFuncName);

    void ReplicateActorListsForConnections_Default(UNetReplicationGraphConnection* ConnectionManager, FGatheredReplicationActorLists& GatheredReplicationListsForConnection, FNetViewerArray& Viewers);

    void ReplicateActorsForConnection(UNetConnection* NetConnection, FPerConnectionActorInfoMap& ConnectionActorInfoMap, UNetReplicationGraphConnection* ConnectionManager, const uint32 FrameNum);

    void ReplicateActorListsForConnections_FastShared(UNetReplicationGraphConnection* ConnectionManager, FGatheredReplicationActorLists& GatheredReplicationListsForConnection, FNetViewerArray& Viewers);

    int64 ReplicateSingleActor_FastShared(AActor* Actor, FConnectionReplicationActorInfo& ConnectionData, FGlobalActorReplicationInfo& GlobalActorInfo, UNetReplicationGraphConnection& ConnectionManager, const uint32 FrameNum);

    int64 ReplicateSingleActor(AActor* Actor, FConnectionReplicationActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalActorInfo, FPerConnectionActorInfoMap& ConnectionActorInfoMap, UNetReplicationGraphConnection& ConnectionManager, const uint32 FrameNum);

    void HandleStarvedActorList(const FPrioritizedRepList& List, int32 StartIdx, FPerConnectionActorInfoMap& ConnectionActorInfoMap, uint32 FrameNum);

    void UpdateActorChannelCloseFrameNum(AActor* Actor, FConnectionReplicationActorInfo& ConnectionData, const FGlobalActorReplicationInfo& GlobalData, const uint32 FrameNum, UNetConnection* NetConnection);

    bool ProcessRemoteFunction(AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, UObject* SubObject);

    void NotifyConnectionSaturated(UNetReplicationGraphConnection& Connection);

protected:
    void AddConnectionGraphNode(UReplicationGraphNode* GraphNode, UNetReplicationGraphConnection* ConnectionManager);

    void RemoveConnectionGraphNode(UReplicationGraphNode* GraphNode, UNetReplicationGraphConnection* ConnectionManager);

private:
    UNetReplicationGraphConnection* CreateClientConnectionManagerInternal(UNetConnection* Connection);
};
