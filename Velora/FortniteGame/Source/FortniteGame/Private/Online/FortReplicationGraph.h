#pragma once

#include "ReplicationGraph.h"

class AFortTeamHealthInfo;
class APlayerState;
class UNetConnection;

class UFortReplicationGraph : public UReplicationGraph
{
public:
    static UClass* StaticClass();

    UReplicationGraphNode*& GetPlayerStateLimiterNode();

    UReplicationGraphNode*& GetReplayPlayerStateLimiterNode();

    TMap<FName, FActorRepListRefView>& GetAlwaysRelevantStreamingLevelActors();

    void InitializeForWorld(UWorld* World);

    void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager);
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
