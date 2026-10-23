#include "pch.h"
#include "FortReplicationGraph.h"
#include "FortTeamActorInterface.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Player/FortPlayerState.h"
#include "Player/FortTeamHealthInfo.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_AlwaysRelevantHealthForTeam"));

    return Class;
}

FActorRepListRefView& UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x20)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantHealthForTeam"))
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
}

TMap<FObjectKey, TArray<TWeakObjectPtr<AFortTeamHealthInfo>>>& UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::GetReplicationActorMap()
{
    return *(TMap<FObjectKey, TArray<TWeakObjectPtr<AFortTeamHealthInfo>>>*)((uint8*)&GetReplicationActorList() - sizeof(TMap<FObjectKey, TArray<TWeakObjectPtr<AFortTeamHealthInfo>>>));
}

AFortTeamHealthInfo*& UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::GetFortTeamHealthInfo()
{
    return *(AFortTeamHealthInfo**)((uint8*)&GetReplicationActorMap() - sizeof(AFortTeamHealthInfo*));
}

void UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UFortReplicationGraphNode_AlwaysRelevantHealthForTeam, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantHealthForTeam"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraphNode_AlwaysRelevantHealthForTeam::StaticClass()));

    auto FortTeamHealthInfo = GetFortTeamHealthInfo();

    if (!FortTeamHealthInfo)
        return;

    auto& ReplicationActorList = GetReplicationActorList();
    ReplicationActorList.RepList.Reset();

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        auto PlayerController = Cast<APlayerController>(Params.GetViewer(Index).InViewer);

        if (!PlayerController)
            continue;

        auto PlayerState = Cast<AFortPlayerState>(PlayerController->GetPlayerState());

        if (!PlayerState)
            continue;

        auto TeamActor = (IFortTeamActorInterface*)PlayerState->GetInterfaceAddress(IFortTeamActorInterface::StaticClass());

        if (TeamActor->GetTeam() != FortTeamHealthInfo->GetTeamNum())
            continue;

        auto& ReplicationActorMap = GetReplicationActorMap();
        const FObjectKey PlayerStateKey(PlayerState);

        if (!ReplicationActorMap.Contains(PlayerStateKey))
            continue;

        const TArray<TWeakObjectPtr<AFortTeamHealthInfo>> TeamHealthInfos = *ReplicationActorMap.Find(PlayerStateKey);

        for (const auto& TeamHealthInfo : TeamHealthInfos)
        {
            if (auto Actor = (AActor*)TeamHealthInfo.Get())
            {
                if (!ReplicationActorList.RepList.Contains(Actor))
                    ReplicationActorList.RepList.Add(Actor);
            }
        }
    }

    if (ReplicationActorList.RepList.Num() > 0)
        Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ReplicationActorList);
}

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
