#pragma once

#include "ReplicationGraph.h"

class AFortTeamHealthInfo;

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
