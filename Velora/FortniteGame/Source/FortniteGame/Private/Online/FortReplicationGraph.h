#pragma once

#include "ReplicationGraph.h"
#include "Containers/Map.h"
#include "ReplicationGraphTypes.h"
#include "UObject/WeakObjectPtr.h"

class AFortTeamHealthInfo;
class APlayerState;
class UNetConnection;

class UFortReplicationGraph : public UReplicationGraph
{
public:
    static UClass* StaticClass();

    UReplicationGraphNode*& GetPlayerStateLimiterNode();

    UReplicationGraphNode*& GetReplayPlayerStateLimiterNode();

    UReplicationGraphNode_GridSpatialization2D*& GetRootGridNode();

    TMap<FName, FActorRepListRefView>& GetAlwaysRelevantStreamingLevelActors();

    void InitializeForWorld(UWorld* World);

    void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager);

    void InitDynamicSpatializationSettings();

private:
    void EnableDynamicSpatialFrequencyNode(bool bEnable);
};

struct FFortAlwaysRelevantActorInfo
{
    bool operator==(const UNetConnection* InConnection) const
    {
        return Connection == InConnection;
    }

    const UNetConnection* Connection;
    AActor* LastPawn;
    AActor* LastTetherPawn;
};

class UFortReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);

    void OnClientLevelVisibilityRemove(FName LevelName);

private:
    uint8* GetAlwaysRelevantStreamingLevelsNeedingReplication();

    int32& GetNumAlwaysRelevantStreamingLevelsNeedingReplication();

    FActorRepListRefView& GetReplicationActorList();

    TArray<FFortAlwaysRelevantActorInfo>& GetPastRelevantActors();

    bool& GetInitializedPlayerState();
};

class UFortReplicationGraphNode_AlwaysRelevantHealthForTeam : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    AFortTeamHealthInfo*& GetFortTeamHealthInfo();

    TMap<FObjectKey, TArray<TWeakObjectPtr<AFortTeamHealthInfo>>>& GetReplicationActorMap();

    FActorRepListRefView& GetReplicationActorList();
};

class UFortReplicationGraphNode_PlayerStateFrequencyLimiter : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void InitNode(UWorld* World);

    void OnPlayerStateAdded(APlayerState* AddedPlayerState);

    void OnPlayerStateRemoved(APlayerState* RemovedPlayerState);

    int32& GetMaxStatesToReplicatePerFrame();

private:
    TMap<APlayerState*, int32>& GetPlayerStateStoredListIndexMap();

    TArray<FActorRepListRefView>& GetPerFramePlayerStateLists();

    int32& GetCurrentFrameListIndex();

    TBitArray<>& GetActiveLists();
};

enum class EFortDynamicNodeType : uint8
{
    WorldStatic,
    WorldDynamic,
    Pawn,
    Visibility,
};

class UFortReplicationGraphNode_DynamicSpatialFrequency : public UReplicationGraphNode_DynamicSpatialFrequency
{
public:
    static UClass* StaticClass();

    static TClassMap<bool>& GetAIClasses();

    static FSettings* GetNodeSettings();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void SetupClassToNodeMap();

protected:
    void GatherActors(const FActorRepListRefView& RepList, FGlobalActorReplicationInfoMap& GlobalMap, FPerConnectionActorInfoMap& ConnectionMap, const FConnectionGatherActorListParameters& Params, UNetConnection* NetConnection);

private:
    EFortDynamicNodeType& GetCurrentDynamicNodeType();

    TMap<EFortDynamicNodeType, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>>& GetDynamicTypeToNodeMap();
};

class UFortReplicationGraphNode_LiveSpectator : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    void GatherBroadcasterActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    FActorRepListRefView& GetBroadcasterRelevancyActorList();

    TArray<FFortAlwaysRelevantActorInfo>& GetPastViewedTargets();
};

class UReplicationGraphNode_FortVolumeGrid : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    bool& GetIsWaitingNewStreamingLevel();

    UReplicationGraphNode*& GetFortVolumeGrid2D();

    UReplicationGraphNode*& GetFortVolumeGlobalRelevancyNode();
};
