#include "pch.h"
#include "FortReplicationGraph.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UReplicationGraphNode_FortVolumeGrid::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ReplicationGraphNode_FortVolumeGrid"));

    return Class;
}

bool& UReplicationGraphNode_FortVolumeGrid::GetIsWaitingNewStreamingLevel()
{
    static auto PendingSpatializedActors = Anchor::Offset(TEXT("UReplicationGraphNode_FortVolumeGrid::PendingSpatializedActors"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .Entry(TEXT("/Script/FortniteGame.Default__ReplicationGraphNode_FortVolumeGrid"))
        .Find("lea r*, [rcx+0x*]")
        .Disp();

    return *(bool*)((uint8*)this + PendingSpatializedActors - sizeof(void*));
}

UReplicationGraphNode*& UReplicationGraphNode_FortVolumeGrid::GetFortVolumeGrid2D()
{
    static auto FortVolumeGrid2D = Anchor::Offset(TEXT("UReplicationGraphNode_FortVolumeGrid::FortVolumeGrid2D"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x28)
        .Entry(TEXT("/Script/FortniteGame.Default__ReplicationGraphNode_FortVolumeGrid"))
        .Find("mov rcx, [rbx+0x*]")
        .Disp();

    return *(UReplicationGraphNode**)((uint8*)this + FortVolumeGrid2D);
}

UReplicationGraphNode*& UReplicationGraphNode_FortVolumeGrid::GetFortVolumeGlobalRelevancyNode()
{
    static auto FortVolumeGlobalRelevancyNode = Anchor::Offset(TEXT("UReplicationGraphNode_FortVolumeGrid::FortVolumeGlobalRelevancyNode"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x28)
        .Entry(TEXT("/Script/FortniteGame.Default__ReplicationGraphNode_FortVolumeGrid"))
        .Find("mov rcx, [rcx+0x*]")
        .Disp();

    return *(UReplicationGraphNode**)((uint8*)this + FortVolumeGlobalRelevancyNode);
}

void UReplicationGraphNode_FortVolumeGrid::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_FortVolumeGrid, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__ReplicationGraphNode_FortVolumeGrid"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_FortVolumeGrid::StaticClass()));

    if (GetIsWaitingNewStreamingLevel())
        return;

    GetFortVolumeGlobalRelevancyNode()->GatherActorListsForConnection(Params);
    GetFortVolumeGrid2D()->GatherActorListsForConnection(Params);
}
