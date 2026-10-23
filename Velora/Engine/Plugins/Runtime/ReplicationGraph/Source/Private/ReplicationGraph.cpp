#include "pch.h"
#include "ReplicationGraph.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/PackageMapClient.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/UnrealMemory.h"
#include "Kismet/GameplayStatics.h"
#include "Net/DataBunch.h"
#include "Net/RepLayout.h"
#include "Templates/Casts.h"
#include "Templates/Sorting.h"
#include <cfloat>
#include "HAL/IConsoleManager.h"
#include "UObject/Class.h"
#include "UObject/CoreNet.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UNetReplicationGraphConnection::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.NetReplicationGraphConnection"));

    return Class;
}

UNetConnection*& UNetReplicationGraphConnection::GetNetConnection()
{
    static auto NetConnection = StaticClass()->FindPropertyByName(FName(TEXT("NetConnection")));

    return *NetConnection->ContainerPtrToValuePtr<UNetConnection*>(this);
}

int32& UNetReplicationGraphConnection::GetConnectionOrderNum()
{
    static auto ConnectionOrderNum = Anchor::Offset(TEXT("UNetReplicationGraphConnection::ConnectionOrderNum"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x20)
        .Find("lea r8, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov [*+0x*], ecx")
        .Disp();

    return *(int32*)((uint8*)this + ConnectionOrderNum);
}

int32& UNetReplicationGraphConnection::GetConnectionId()
{
    static auto ConnectionId = Anchor::Offset(TEXT("UNetReplicationGraphConnection::ConnectionId"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x20)
        .Find("lea r8, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov [*+0x*], ecx", 2)
        .Disp();

    return *(int32*)((uint8*)this + ConnectionId);
}

FGlobalActorReplicationInfoMap*& UNetReplicationGraphConnection::GetGlobalMap()
{
    static auto GlobalMap = Anchor::Offset(TEXT("UNetReplicationGraphConnection::ActorInfoMap::GlobalMap"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"), [](const Anchor::FCode& Entry) { return Entry.Has("mov ecx, 0x30; call 0x*; mov *, rax; test rax, rax; jz 0x*; mov rcx, [*+0x*]"); })
        .Find("mov ecx, 0x30; call 0x*")
        .Find("mov rcx, [*+0x*]")
        .Disp();

    return *(FGlobalActorReplicationInfoMap**)((uint8*)this + GlobalMap);
}

FPerConnectionActorInfoMap& UNetReplicationGraphConnection::GetActorInfoMap()
{
    return *(FPerConnectionActorInfoMap*)((uint8*)&GetGlobalMap() - offsetof(FPerConnectionActorInfoMap, GlobalMap));
}

TArray<FCachedDestructInfo>& UNetReplicationGraphConnection::GetPendingDestructInfoList()
{
    static auto PendingDestructInfoList = Anchor::Offset(TEXT("UNetReplicationGraphConnection::PendingDestructInfoList"))
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x20)
        .Find("lea rcx, [rdi+0x*]")
        .Disp();

    return *(TArray<FCachedDestructInfo>*)((uint8*)this + PendingDestructInfoList);
}

TSet<FActorDestructionInfo*>& UNetReplicationGraphConnection::GetTrackedDestructionInfoPtrs()
{
    static auto TrackedDestructionInfoPtrs = Anchor::Offset(TEXT("UNetReplicationGraphConnection::TrackedDestructionInfoPtrs"))
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x20)
        .Find("lea rbx, [rcx+0x*]")
        .Disp();

    return *(TSet<FActorDestructionInfo*>*)((uint8*)this + TrackedDestructionInfoPtrs);
}

TMap<FName, TMulticastDelegate<void, FName, UWorld*>>& UNetReplicationGraphConnection::GetOnClientVisibleLevelNameAddMap()
{
    static auto OnClientVisibleLevelNameAddMap = Anchor::Offset(TEXT("UNetReplicationGraphConnection::OnClientVisibleLevelNameAddMap"))
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x28)
        .Find("mov rcx, [rsi+0x*]")
        .Disp();

    return *(TMap<FName, TMulticastDelegate<void, FName, UWorld*>>*)((uint8*)this + OnClientVisibleLevelNameAddMap);
}

TArray<FLastLocationGatherInfo>& UNetReplicationGraphConnection::GetLastGatherLocations()
{
    return *(TArray<FLastLocationGatherInfo>*)(((uintptr_t)&GetConnectionId() + sizeof(int32) + alignof(void*) - 1) & ~(uintptr_t)(alignof(void*) - 1));
}

TMulticastDelegate<void, UNetReplicationGraphConnection*, FPrioritizedRepList*>& UNetReplicationGraphConnection::GetOnPostReplicatePrioritizeLists()
{
    return *(TMulticastDelegate<void, UNetReplicationGraphConnection*, FPrioritizedRepList*>*)(&GetActorInfoMap() + 1);
}

int32& UNetReplicationGraphConnection::GetQueuedBitsForActorDiscovery()
{
    return *(int32*)(&GetLastGatherLocations() + 1);
}

TArray<FCachedDestructInfo>& UNetReplicationGraphConnection::GetOutOfRangeDestroyedActors()
{
    return *(&GetPendingDestructInfoList() - 1);
}

TArray<UReplicationGraphNode*>& UNetReplicationGraphConnection::GetConnectionGraphNodes()
{
    static auto ConnectionGraphNodes = StaticClass()->FindPropertyByName(FName(TEXT("ConnectionGraphNodes")));

    return *ConnectionGraphNodes->ContainerPtrToValuePtr<TArray<UReplicationGraphNode*>>(this);
}

void UNetReplicationGraphConnection::SetActorNotDormantOnConnection(AActor* InActor)
{
    static auto SetActorNotDormantOnConnection = Anchor::Function<void (*)(UNetReplicationGraphConnection*, AActor*)>(TEXT("UNetReplicationGraphConnection::SetActorNotDormantOnConnection"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .Find("mov rdx, [rbp+0x*]; mov rcx, [rbx]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    SetActorNotDormantOnConnection(this, InActor);
}

bool UNetReplicationGraphConnection::PrepareForReplication()
{
    auto NetConnection = GetNetConnection();
    auto PlayerController = NetConnection->GetPlayerController();

    NetConnection->SetViewTarget(PlayerController ? PlayerController->GetViewTarget() : NetConnection->GetOwningActor());

    for (auto Child : NetConnection->GetChildren())
    {
        auto ChildPlayerController = Child->GetPlayerController();

        Child->SetViewTarget(ChildPlayerController ? ChildPlayerController->GetViewTarget() : Child->GetOwningActor());
    }

    return NetConnection->GetState() != USOCK_Closed && NetConnection->GetViewTarget() != nullptr;
}

void UNetReplicationGraphConnection::GetClientVisibleLevelNames(TSet<FName>& OutLevelNames)
{
    static auto GetClientVisibleLevelNames = Anchor::VirtualFunction<void (*)(UNetReplicationGraphConnection*, TSet<FName>&)>(TEXT("UNetReplicationGraphConnection::GetClientVisibleLevelNames"))
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x48)
        .Get();

    GetClientVisibleLevelNames(this, OutLevelNames);
}

void UNetReplicationGraphConnection::UpdateGatherLocationsForConnection(const FNetViewerArray& ConnectionViewers, const FReplicationGraphDestructionSettings& DestructionSettings)
{
    auto& LastGatherLocations = GetLastGatherLocations();

    for (const FNetViewer& CurViewer : ConnectionViewers)
    {
        if (CurViewer.Connection != nullptr)
        {
            if (auto LastInfoForViewer = LastGatherLocations.FindByKey(CurViewer.Connection))
                OnUpdateViewerLocation(LastInfoForViewer, CurViewer, DestructionSettings);
            else
                LastGatherLocations.Emplace(CurViewer.Connection, CurViewer.GetViewLocation());
        }
    }

    LastGatherLocations.RemoveAll([](const FLastLocationGatherInfo& CurGatherInfo)
    {
        return CurGatherInfo.Connection == nullptr;
    });
}

void UNetReplicationGraphConnection::OnUpdateViewerLocation(FLastLocationGatherInfo* LocationInfo, const FNetViewer& Viewer, const FReplicationGraphDestructionSettings& DestructionSettings)
{
    const bool bIgnoreDistanceCheck = DestructionSettings.OutOfRangeDistanceCheckThresholdSquared == 0.0f;
    const FVector ViewLocation = Viewer.GetViewLocation();
    const FVector LastOutOfRangeLocationCheck = LocationInfo->GetLastOutOfRangeLocationCheck();
    const float OutOfRangeDistanceSquared = (ViewLocation.X - LastOutOfRangeLocationCheck.X) * (ViewLocation.X - LastOutOfRangeLocationCheck.X) + (ViewLocation.Y - LastOutOfRangeLocationCheck.Y) * (ViewLocation.Y - LastOutOfRangeLocationCheck.Y);

    if (bIgnoreDistanceCheck || OutOfRangeDistanceSquared > DestructionSettings.OutOfRangeDistanceCheckThresholdSquared)
    {
        LocationInfo->SetLastOutOfRangeLocationCheck(ViewLocation);

        auto& OutOfRangeDestroyedActors = GetOutOfRangeDestroyedActors();

        for (int32 Index = OutOfRangeDestroyedActors.Num() - 1; Index >= 0; --Index)
        {
            FCachedDestructInfo& CachedInfo = OutOfRangeDestroyedActors[Index];
            const FVector CachedPosition = CachedInfo.GetCachedPosition();
            const float ActorDistSquared = (CachedPosition.X - ViewLocation.X) * (CachedPosition.X - ViewLocation.X) + (CachedPosition.Y - ViewLocation.Y) * (CachedPosition.Y - ViewLocation.Y);

            if (ActorDistSquared < DestructionSettings.DestructInfoMaxDistanceSquared)
            {
                GetPendingDestructInfoList().Emplace(CachedInfo);
                OutOfRangeDestroyedActors.RemoveAtSwap(Index, 1, false);
            }
        }
    }

    LocationInfo->SetLastLocation(ViewLocation);
}

int64 UNetReplicationGraphConnection::ReplicateDestructionInfos(const FNetViewerArray& Viewers, const float DestructInfoMaxDistanceSquared)
{
    int64 NumBits = 0;
    auto& PendingDestructInfoList = GetPendingDestructInfoList();

    for (int32 idx = PendingDestructInfoList.Num() - 1; idx >= 0; --idx)
    {
        FCachedDestructInfo& Info = PendingDestructInfoList[idx];
        FActorDestructionInfo* DestructInfo = Info.DestructionInfo;
        bool bSendDestructionInfo = false;

        if (!DestructInfo->GetIgnoreDistanceCulling())
        {
            const FVector CachedPosition = Info.GetCachedPosition();

            for (const FNetViewer& CurViewer : Viewers)
            {
                const FVector ViewLocation = CurViewer.GetViewLocation();
                const float DistSquared = (CachedPosition.X - ViewLocation.X) * (CachedPosition.X - ViewLocation.X) + (CachedPosition.Y - ViewLocation.Y) * (CachedPosition.Y - ViewLocation.Y);

                if (DistSquared < DestructInfoMaxDistanceSquared)
                {
                    bSendDestructionInfo = true;
                    break;
                }
            }
        }

        if (bSendDestructionInfo || DestructInfo->GetIgnoreDistanceCulling())
        {
            auto NetConnection = GetNetConnection();

            if (NetConnection && NetConnection->GetDriver())
            {
                NumBits += NetConnection->GetDriver()->SendDestructionInfo(NetConnection, DestructInfo);

                PendingDestructInfoList.RemoveAtSwap(idx, 1, false);
                GetTrackedDestructionInfoPtrs().Remove(DestructInfo);
            }
        }
        else
        {
            GetOutOfRangeDestroyedActors().Emplace(Info);
            PendingDestructInfoList.RemoveAtSwap(idx, 1, false);
        }
    }

    return NumBits;
}

int64 UNetReplicationGraphConnection::ReplicateDormantDestructionInfos()
{
    int64 NumBits = 0;
    auto NetConnection = GetNetConnection();

    if (NetConnection && NetConnection->GetDriver())
    {
        for (const FCachedDormantDestructInfo& Info : GetPendingDormantDestructList())
        {
            alignas(16) uint8 DestructInfoStorage[0x80] = {};
            auto& DestructInfo = *(FActorDestructionInfo*)DestructInfoStorage;

            DestructInfo.GetNetGUID() = Info.NetGUID;
            DestructInfo.GetLevel() = Info.Level;
            DestructInfo.GetObjOuter() = Info.ObjOuter;
            DestructInfo.GetPathName() = Info.PathName;
            DestructInfo.GetReason() = EChannelCloseReason::Relevancy;

            NumBits += NetConnection->GetDriver()->SendDestructionInfo(NetConnection, &DestructInfo);

            DestructInfo.GetPathName().~FString();
        }

        GetPendingDormantDestructList().Reset();
        GetTrackedDormantDestructionInfos().Reset();
    }

    return NumBits;
}

FActorRepListRefView& UNetReplicationGraphConnection::GetPrevDormantActorListForNode(const UReplicationGraphNode* GridNode)
{
    auto QueuedBitsForActorDiscovery = (uintptr_t)&GetLastGatherLocations() + sizeof(TArray<FLastLocationGatherInfo>);
    auto& PrevDormantActorListPerNode = *(TMap<FObjectKey, FActorRepListRefView>*)((QueuedBitsForActorDiscovery + sizeof(int32) + alignof(void*) - 1) & ~(uintptr_t)(alignof(void*) - 1));

    return PrevDormantActorListPerNode.FindOrAdd(FObjectKey(GridNode));
}

TArray<FCachedDormantDestructInfo>& UNetReplicationGraphConnection::GetPendingDormantDestructList()
{
    return *(TArray<FCachedDormantDestructInfo>*)((uint8*)&GetTrackedDestructionInfoPtrs() + sizeof(TSet<FActorDestructionInfo*>));
}

TSet<FNetworkGUID>& UNetReplicationGraphConnection::GetTrackedDormantDestructionInfos()
{
    return *(TSet<FNetworkGUID>*)((uint8*)&GetPendingDormantDestructList() + sizeof(TArray<FCachedDormantDestructInfo>));
}

void UNetReplicationGraphConnection::NotifyActorChannelAdded(AActor* Actor, UActorChannel* Channel)
{
    DETOUR(UNetReplicationGraphConnection, NotifyActorChannelAdded, Anchor::Hook()
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), -0x10)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), "ret*")
        .Optional()
        .InChildVTables(UNetReplicationGraphConnection::StaticClass()));

    auto& ActorInfoMap = GetActorInfoMap();
    auto& ActorInfo = ActorInfoMap.FindOrAdd(Actor);

    if (ActorInfo.Channel && Channel != ActorInfo.Channel)
        ActorInfoMap.RemoveChannel(ActorInfo.Channel);

    ActorInfo.Channel = Channel;
    ActorInfoMap.AddChannel(Actor, Channel);
}

void UNetReplicationGraphConnection::NotifyAddDestructionInfo(FActorDestructionInfo* DestructInfo)
{
    DETOUR(UNetReplicationGraphConnection, NotifyAddDestructionInfo, Anchor::Hook()
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x8)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), "ret*")
        .Optional()
        .InChildVTables(UNetReplicationGraphConnection::StaticClass()));

    const FName StreamingLevelName = DestructInfo->GetStreamingLevelName();

    if (StreamingLevelName != NAME_None)
    {
        auto& ClientVisibleLevelNames = GetNetConnection()->GetClientVisibleLevelNames();

        if (!(FName::IsOutlineNumber() ? ((TSet<FNameEntryId>&)ClientVisibleLevelNames).Contains(StreamingLevelName.GetComparisonIndex()) : ClientVisibleLevelNames.Contains(StreamingLevelName)))
            return;
    }

    auto& TrackedDestructionInfoPtrs = GetTrackedDestructionInfoPtrs();

    if (TrackedDestructionInfoPtrs.Contains(DestructInfo))
        return;

    TrackedDestructionInfoPtrs.Add(DestructInfo);
    GetPendingDestructInfoList().Emplace(DestructInfo);
}

void UNetReplicationGraphConnection::NotifyAddDormantDestructionInfo(AActor* Actor)
{
    DETOUR(UNetReplicationGraphConnection, NotifyAddDormantDestructionInfo, Anchor::Hook()
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x10)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), "ret*")
        .Optional()
        .InChildVTables(UNetReplicationGraphConnection::StaticClass()));

    auto NetConnection = GetNetConnection();

    if (!Actor || !NetConnection || !NetConnection->GetDriver() || !NetConnection->GetDriver()->GetGuidCache().IsValid())
        return;

    ULevel* Level = Actor->GetLevel();

    if (Level && Level->IsPersistentLevel())
    {
        const FName StreamingLevelName = Level->GetOutermost()->GetFName();
        auto& ClientVisibleLevelNames = NetConnection->GetClientVisibleLevelNames();

        if (!(FName::IsOutlineNumber() ? ((TSet<FNameEntryId>&)ClientVisibleLevelNames).Contains(StreamingLevelName.GetComparisonIndex()) : ClientVisibleLevelNames.Contains(StreamingLevelName)))
            return;
    }

    const FNetworkGUID NetGUID = NetConnection->GetDriver()->GetGuidCache()->GetNetGUID(Actor);

    if (!NetGUID.IsValid() || NetGUID.IsDefault())
        return;

    auto& TrackedDormantDestructionInfos = GetTrackedDormantDestructionInfos();

    if (TrackedDormantDestructionInfos.Contains(NetGUID))
        return;

    TrackedDormantDestructionInfos.Add(NetGUID);

    auto& Info = *new(GetPendingDormantDestructList()) FCachedDormantDestructInfo();

    Info.NetGUID = NetGUID;
    Info.Level = Level;
    Info.ObjOuter = Actor->GetOuter();
    Info.PathName = Actor->GetName();
}

void UReplicationGraphNode::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    static auto GatherActorListsForConnection = Anchor::VirtualFunction<void (*)(UReplicationGraphNode*, const FConnectionGatherActorListParameters&)>(TEXT("UReplicationGraphNode::GatherActorListsForConnection"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Get();

    GatherActorListsForConnection(this, Params);
}

void UReplicationGraphNode::PrepareForReplication()
{
    static auto PrepareForReplication = Anchor::VirtualFunction<void (*)(UReplicationGraphNode*)>(TEXT("UReplicationGraphNode::PrepareForReplication"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .Get();

    PrepareForReplication(this);
}

void UReplicationGraphNode::NotifyAddNetworkActor(const FNewReplicatedActorInfo& ActorInfo)
{
    static auto NotifyAddNetworkActor = Anchor::VirtualFunction<void (*)(UReplicationGraphNode*, const FNewReplicatedActorInfo&)>(TEXT("UReplicationGraphNode::NotifyAddNetworkActor"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Get();

    NotifyAddNetworkActor(this, ActorInfo);
}

void UReplicationGraphNode::TearDown()
{
    static auto TearDown = Anchor::VirtualFunction<void (*)(UReplicationGraphNode*)>(TEXT("UReplicationGraphNode::TearDown"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x20)
        .Get();

    TearDown(this);
}

void UReplicationGraphNode::CleanChildNodes()
{
    auto& AllChildNodes = GetAllChildNodes();

    for (int32 Index = 0; Index < AllChildNodes.Num();)
    {
        auto ChildNode = AllChildNodes[Index];

        if (!ChildNode || (ChildNode->GetFlags() & 0x60000000))
            AllChildNodes.RemoveAtSwap(Index, 1, false);
        else
            Index++;
    }
}

TArray<UReplicationGraphNode*>& UReplicationGraphNode::GetAllChildNodes()
{
    static auto AllChildNodes = Anchor::Offset(TEXT("UReplicationGraphNode::AllChildNodes"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Find("mov rdi, [rbx+0x*]")
        .Disp();

    return *(TArray<UReplicationGraphNode*>*)((uint8*)this + AllChildNodes);
}

TSharedPtr<FReplicationGraphGlobalData, ESPMode::ThreadSafe>& UReplicationGraphNode::GetGraphGlobals()
{
    static auto GraphGlobals = Anchor::Offset(TEXT("UReplicationGraphNode::GraphGlobals"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x28)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_DormancyNode"))
        .Find("cmp qword ptr [rcx+0x*], 0x0")
        .Disp();

    return *(TSharedPtr<FReplicationGraphGlobalData, ESPMode::ThreadSafe>*)((uint8*)this + GraphGlobals);
}

FActorRepListRefView& UReplicationGraphNode_ActorList::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UReplicationGraphNode_ActorList::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Find("lea rdx, [rcx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
}

FStreamingLevelActorListCollection& UReplicationGraphNode_ActorList::GetStreamingLevelCollection()
{
    static auto StreamingLevelCollection = Anchor::Offset(TEXT("UReplicationGraphNode_ActorList::StreamingLevelCollection"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Find("lea rcx, [rbx+0x*]; mov rdx, rsi")
        .Disp();

    return *(FStreamingLevelActorListCollection*)((uint8*)this + StreamingLevelCollection);
}

UClass* UReplicationGraphNode_ConnectionDormancyNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_ConnectionDormancyNode"));

    return Class;
}

FObjectKey& UReplicationGraphNode_ConnectionDormancyNode::GetConnectionOwner()
{
    return *(FObjectKey*)((uint8*)&GetStreamingLevelCollection() + sizeof(FStreamingLevelActorListCollection));
}

uint32& UReplicationGraphNode_ConnectionDormancyNode::GetLastGatheredFrame()
{
    return *(uint32*)((uint8*)&GetConnectionOwner() + sizeof(FObjectKey));
}

int32& UReplicationGraphNode_ConnectionDormancyNode::GetTrickleStartCounter()
{
    return *(int32*)((uint8*)&GetLastGatheredFrame() + sizeof(uint32));
}

FStreamingLevelActorListCollection& UReplicationGraphNode_ConnectionDormancyNode::GetRemovedStreamingLevelActorListCollection()
{
    return *(FStreamingLevelActorListCollection*)((uint8*)&GetTrickleStartCounter() + sizeof(int32));
}

void UReplicationGraphNode_ConnectionDormancyNode::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_ConnectionDormancyNode, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ConnectionDormancyNode"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_ConnectionDormancyNode::StaticClass()));

    GetLastGatheredFrame() = Params.GetReplicationFrameNum();

    ConditionalGatherDormantActorsForConnection(GetReplicationActorList(), Params, nullptr);

    auto& StreamingLevelLists = GetStreamingLevelCollection().StreamingLevelLists;
    auto& RemovedStreamingLevelLists = GetRemovedStreamingLevelActorListCollection().StreamingLevelLists;

    for (int32 idx = StreamingLevelLists.Num() - 1; idx >= 0; --idx)
    {
        auto& StreamingList = StreamingLevelLists[idx];

        if (StreamingList.ReplicationActorList.RepList.Num() <= 0)
        {
            StreamingLevelLists.RemoveAtSwap(idx, 1, false);
            continue;
        }

        if (Params.CheckClientVisibilityForLevel(StreamingList.StreamingLevelName))
        {
            const FName StreamingLevelName = TGameLayout<FName>::Load(&StreamingList.StreamingLevelName);
            auto RemoveList = RemovedStreamingLevelLists.FindByKey(StreamingLevelName);

            if (!RemoveList)
            {
                RemoveList = new(RemovedStreamingLevelLists) FStreamingLevelActorListCollection::FStreamingLevelActors(StreamingLevelName);
                Params.GetConnectionManager()->GetOnClientVisibleLevelNameAddMap().FindOrAdd(StreamingLevelName).AddUObject(this, &UReplicationGraphNode_ConnectionDormancyNode::OnClientVisibleLevelNameAdd);
            }

            ConditionalGatherDormantActorsForConnection(StreamingList.ReplicationActorList, Params, &RemoveList->ReplicationActorList);
        }
    }
}

void UReplicationGraphNode_ConnectionDormancyNode::ConditionalGatherDormantActorsForConnection(FActorRepListRefView& ConnectionList, const FConnectionGatherActorListParameters& Params, FActorRepListRefView* RemovedList)
{
    auto& ConnectionActorInfoMap = Params.GetConnectionManager()->GetActorInfoMap();
    auto GlobalActorReplicationInfoMap = GetGraphGlobals()->GlobalActorReplicationInfoMap;
    bool bShouldTrickle = GetTrickleStartCounter() == 0;

    for (int32 idx = ConnectionList.RepList.Num() - 1; idx >= 0; --idx)
    {
        AActor* Actor = ConnectionList.RepList[idx];
        auto& ConnectionActorInfo = ConnectionActorInfoMap.FindOrAdd(Actor);

        if (ConnectionActorInfo.bDormantOnConnection)
        {
            if (ConnectionActorInfo.CullDistanceSquared <= 0.f)
            {
                auto& GlobalInfo = GlobalActorReplicationInfoMap->Get(Actor);
                ConnectionActorInfo.SetCullDistanceSquared(GlobalInfo.GetCullDistanceSquared());
            }

            ConnectionList.RepList.RemoveAtSwap(idx);

            if (RemovedList)
                RemovedList->RepList.Add(Actor);

            bShouldTrickle = false;
        }
        else if (bShouldTrickle)
        {
            ConnectionActorInfo.SetCullDistanceSquared(0.f);
            bShouldTrickle = false;
        }
    }

    if (ConnectionList.RepList.Num() > 0)
    {
        Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ConnectionList);

        if (GetTrickleStartCounter() > 0)
            GetTrickleStartCounter()--;
    }
}

bool ContainsReverse(const FActorRepListRefView& List, AActor* Actor)
{
    for (int32 idx = List.RepList.Num() - 1; idx >= 0; --idx)
    {
        if (List.RepList[idx] == Actor)
            return true;
    }

    return false;
}

void UReplicationGraphNode_ConnectionDormancyNode::NotifyActorDormancyFlush(AActor* Actor)
{
    const FName StreamingLevelName = FNewReplicatedActorInfo::GetStreamingLevelNameOfActor(Actor);

    if (StreamingLevelName == FName(NAME_None))
    {
        if (!ContainsReverse(GetReplicationActorList(), Actor))
            GetReplicationActorList().RepList.Add(Actor);
    }
    else
    {
        auto& StreamingLevelLists = GetStreamingLevelCollection().StreamingLevelLists;
        auto Item = StreamingLevelLists.FindByKey(StreamingLevelName);

        if (!Item)
        {
            Item = new(StreamingLevelLists) FStreamingLevelActorListCollection::FStreamingLevelActors(StreamingLevelName);
            Item->ReplicationActorList.RepList.Add(Actor);
        }
        else if (!ContainsReverse(Item->ReplicationActorList, Actor))
        {
            Item->ReplicationActorList.RepList.Add(Actor);
        }

        if (auto RemoveList = GetRemovedStreamingLevelActorListCollection().StreamingLevelLists.FindByKey(StreamingLevelName))
            RemoveList->ReplicationActorList.RepList.RemoveSwap(Actor);
    }
}

void UReplicationGraphNode_ConnectionDormancyNode::OnClientVisibleLevelNameAdd(FName LevelName, UWorld* World)
{
    auto RemoveList = GetRemovedStreamingLevelActorListCollection().StreamingLevelLists.FindByKey(LevelName);

    if (!RemoveList)
        return;

    auto& StreamingLevelLists = GetStreamingLevelCollection().StreamingLevelLists;
    auto AddList = StreamingLevelLists.FindByKey(LevelName);

    if (!AddList)
        AddList = new(StreamingLevelLists) FStreamingLevelActorListCollection::FStreamingLevelActors(LevelName);

    AddList->ReplicationActorList.RepList.Append(RemoveList->ReplicationActorList.RepList);
    RemoveList->ReplicationActorList.RepList.Reset();
}

UClass* UReplicationGraphNode_DormancyNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_DormancyNode"));

    return Class;
}

TArray<TPair<FObjectKey, UReplicationGraphNode_ConnectionDormancyNode*>>& UReplicationGraphNode_DormancyNode::GetConnectionNodes()
{
    return *(TArray<TPair<FObjectKey, UReplicationGraphNode_ConnectionDormancyNode*>>*)((uint8*)&GetStreamingLevelCollection() + sizeof(FStreamingLevelActorListCollection));
}

void UReplicationGraphNode_DormancyNode::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_DormancyNode, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_DormancyNode"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_DormancyNode::StaticClass()));

    int32 NumViewersAboveMaxZ = 0;

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        if (Params.GetViewer(Index).GetViewLocation().Z > (UE::LWC::IsEnabled() ? 8796093022208.0 : 2097152.0))
            ++NumViewersAboveMaxZ;
    }

    if (Params.GetNumViewers() <= NumViewersAboveMaxZ)
        return;

    auto ConnectionNode = GetExistingConnectionNode(Params);

    if (!ConnectionNode)
        ConnectionNode = CreateConnectionNode(Params);

    ConnectionNode->GatherActorListsForConnection(Params);
}

UReplicationGraphNode_ConnectionDormancyNode* UReplicationGraphNode_DormancyNode::GetExistingConnectionNode(const FConnectionGatherActorListParameters& Params)
{
    const FObjectKey RepGraphConnection(Params.GetConnectionManager());
    auto& ConnectionNodes = GetConnectionNodes();
    int32 Start = 0;

    for (int32 Size = ConnectionNodes.Num(); Size > 0;)
    {
        const int32 LeftoverSize = Size % 2;
        Size = Size / 2;

        const int32 CheckIndex = Start + Size;
        Start = ConnectionNodes[CheckIndex].Key < RepGraphConnection ? CheckIndex + LeftoverSize : Start;
    }

    return Start < ConnectionNodes.Num() && !(RepGraphConnection < ConnectionNodes[Start].Key) ? ConnectionNodes[Start].Value : nullptr;
}

void UReplicationGraphNode_DormancyNode::ConditionalGatherDormantDynamicActors(FActorRepListRefView& RepList, const FConnectionGatherActorListParameters& Params, FActorRepListRefView* RemovedList, bool bEnforceReplistUniqueness, FActorRepListRefView* RemoveFromList)
{
    auto GatherDormantDynamicActorsForList = [&](const FActorRepListRefView& InReplicationActorList)
    {
        for (AActor* Actor : InReplicationActorList.RepList)
        {
            if (!Actor || Actor->IsNetStartupActor())
                continue;

            auto InfoPtr = Params.GetConnectionManager()->GetActorInfoMap().ActorMap.Find(Actor);
            auto Info = InfoPtr ? InfoPtr->Get() : nullptr;

            if (!Info || (!Info->bDormantOnConnection && !Info->Channel))
                continue;

            if (RemovedList && RemovedList->RepList.Contains(Actor))
                continue;

            if (RemoveFromList && RemoveFromList->RepList.RemoveSwap(Actor) > 0)
                Info->bGridSpatilization_AlreadyDormant = false;

            if (bEnforceReplistUniqueness)
            {
                if (Info->bGridSpatilization_AlreadyDormant)
                    continue;

                Info->bGridSpatilization_AlreadyDormant = true;
            }

            RepList.RepList.AddUnique(Actor);
        }
    };

    GatherDormantDynamicActorsForList(GetReplicationActorList());

    for (const auto& StreamingList : GetStreamingLevelCollection().StreamingLevelLists)
    {
        if (Params.CheckClientVisibilityForLevel(StreamingList.StreamingLevelName))
            GatherDormantDynamicActorsForList(StreamingList.ReplicationActorList);
    }
}

UReplicationGraphNode_ConnectionDormancyNode* UReplicationGraphNode_DormancyNode::CreateConnectionNode(const FConnectionGatherActorListParameters& Params)
{
    const FObjectKey RepGraphConnection(Params.GetConnectionManager());
    auto ConnectionNode = CreateChildNode<UReplicationGraphNode_ConnectionDormancyNode>();
    auto& ConnectionNodes = GetConnectionNodes();
    int32 Start = 0;

    for (int32 Size = ConnectionNodes.Num(); Size > 0;)
    {
        const int32 LeftoverSize = Size % 2;
        Size = Size / 2;

        const int32 CheckIndex = Start + Size;
        Start = ConnectionNodes[CheckIndex].Key < RepGraphConnection ? CheckIndex + LeftoverSize : Start;
    }

    if (Start >= ConnectionNodes.Num() || RepGraphConnection < ConnectionNodes[Start].Key)
    {
        ConnectionNodes.AddUninitialized(1);
        memmove(ConnectionNodes.GetData() + Start + 1, ConnectionNodes.GetData() + Start, (ConnectionNodes.Num() - 1 - Start) * sizeof(TPair<FObjectKey, UReplicationGraphNode_ConnectionDormancyNode*>));
    }

    ConnectionNodes[Start].Key = RepGraphConnection;
    ConnectionNodes[Start].Value = ConnectionNode;

    if (GetReplicationActorList().RepList.Num() > 0)
        ConnectionNode->GetReplicationActorList().RepList = GetReplicationActorList().RepList;

    ConnectionNode->GetStreamingLevelCollection().DeepCopyFrom(GetStreamingLevelCollection());
    ConnectionNode->GetConnectionOwner() = RepGraphConnection;
    ConnectionNode->GetLastGatheredFrame() = Params.GetReplicationFrameNum();

    return ConnectionNode;
}

UClass* UReplicationGraphNode_GridCell::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_GridCell"));

    return Class;
}

void UReplicationGraphNode_GridCell::RemoveDynamicActor(const FNewReplicatedActorInfo& ActorInfo)
{
    static auto RemoveDynamicActor = Anchor::Function<void (*)(UReplicationGraphNode_GridCell*, const FNewReplicatedActorInfo&)>(TEXT("UReplicationGraphNode_GridCell::RemoveDynamicActor"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 3)
        .Target()
        .Get();

    RemoveDynamicActor(this, ActorInfo);
}

UReplicationGraphNode* UReplicationGraphNode_GridCell::GetDynamicNode()
{
    static auto DynamicNode = Anchor::Offset(TEXT("UReplicationGraphNode_GridCell::DynamicNode"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 3)
        .Target()
        .Find("cmp qword ptr [rcx+0x*], 0x0")
        .Or()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 3)
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("cmp qword ptr [rcx+0x*], 0x0")
        .Disp();

    static auto CreateDynamicNodeOverride = Anchor::Offset(TEXT("UReplicationGraphNode_GridCell::CreateDynamicNodeOverride"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 3)
        .Target()
        .Has("cmp qword ptr [rcx+0x*], 0x0")
        .Find("mov rdi, [rcx+0x*]")
        .Or()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 3)
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("mov rdi, [rcx+0x*]")
        .Disp();

    auto& Node = *(UReplicationGraphNode**)((uint8*)this + DynamicNode);

    if (!Node)
    {
        auto Override = (uint8*)this + CreateDynamicNodeOverride;
        auto Callable = *(UReplicationGraphNode* (**)(void*, UReplicationGraphNode_GridCell*&))Override;

        if (Callable)
        {
            auto Storage = Override + 0x10;
            auto OwnedObject = *(uint8**)Storage ? *(uint8**)Storage : Storage + 0x10;
            auto GetAddress = (*(void* (***)(void*))OwnedObject)[1];
            UReplicationGraphNode_GridCell* Self = this;

            Node = Callable(GetAddress(OwnedObject), Self);
        }
        else
        {
            Node = CreateChildNode<UReplicationGraphNode_ActorListFrequencyBuckets>();
        }
    }

    return Node;
}

void UReplicationGraphNode_GridCell::AddDynamicActor(const FNewReplicatedActorInfo& ActorInfo)
{
    GetDynamicNode()->NotifyAddNetworkActor(ActorInfo);
}

UReplicationGraphNode_DormancyNode* UReplicationGraphNode_GridCell::GetDormancyNode()
{
    static auto GetDormancyNode = Anchor::Function<UReplicationGraphNode_DormancyNode* (*)(UReplicationGraphNode_GridCell*)>(TEXT("UReplicationGraphNode_GridCell::GetDormancyNode"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    return GetDormancyNode(this);
}

UClass* UReplicationGraphNode_GridSpatialization2D::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_GridSpatialization2D"));

    return Class;
}

UReplicationGraphNode_GridSpatialization2D::FActorCellInfo UReplicationGraphNode_GridSpatialization2D::GetCellInfoForActor(AActor* Actor, const FVector& Location3D, float CullDistance)
{
    static auto GetCellInfoForActor = Anchor::Function<FActorCellInfo* (*)(UReplicationGraphNode_GridSpatialization2D*, FActorCellInfo*, AActor*, const TGameValue<FVector>&, float)>(TEXT("UReplicationGraphNode_GridSpatialization2D::GetCellInfoForActor"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    FActorCellInfo CellInfo;

    return *GetCellInfoForActor(this, &CellInfo, Actor, TGameValue<FVector>(Location3D), CullDistance);
}

void UReplicationGraphNode_GridSpatialization2D::GetGridNodesForActor(AActor* Actor, const FActorCellInfo& CellInfo, TArray<UReplicationGraphNode_GridCell*>& OutNodes)
{
    static auto GetGridNodesForActor = Anchor::Function<void (*)(UReplicationGraphNode_GridSpatialization2D*, AActor*, const FActorCellInfo&, TArray<UReplicationGraphNode_GridCell*>&)>(TEXT("UReplicationGraphNode_GridSpatialization2D::GetGridNodesForActor"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Get();

    GetGridNodesForActor(this, Actor, CellInfo, OutNodes);
}

float& UReplicationGraphNode_GridSpatialization2D::GetCellSize()
{
    static auto CellSize = Anchor::Offset(TEXT("UReplicationGraphNode_GridSpatialization2D::CellSize"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*")
        .Target()
        .Find("mov cl, [rbx+0x*]")
        .Find("movss xmm0, dword ptr [rbx+0x*]")
        .Or()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 2)
        .Target()
        .Find("mov cl, [rbx+0x*]")
        .Find("movss xmm0, dword ptr [rbx+0x*]")
        .Disp();

    return *(float*)((uint8*)this + CellSize);
}

uint8* UReplicationGraphNode_GridSpatialization2D::GetSpatialBias()
{
    return (uint8*)&GetCellSize() + TLWCGameLayout<float, double>::Size();
}

float& UReplicationGraphNode_GridSpatialization2D::GetConnectionMaxZ()
{
    return *(float*)(GetSpatialBias() + 2 * TLWCGameLayout<float, double>::Size());
}

uint8* UReplicationGraphNode_GridSpatialization2D::GetCreateCellNodeOverride()
{
    return (uint8*)((((uintptr_t)&GetConnectionMaxZ() + sizeof(float)) + 0xF) & ~(uintptr_t)0xF);
}

uint8* UReplicationGraphNode_GridSpatialization2D::GetGridBounds()
{
    static auto IsValid = Anchor::Offset(TEXT("UReplicationGraphNode_GridSpatialization2D::GridBounds::IsValid"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("cmp byte ptr [rcx+0x*], 0x0")
        .Or()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*")
        .Target()
        .Find("cmp [rcx+0x*], al")
        .Disp();

    return (uint8*)this + IsValid - 2 * TGameLayout<FVector>::Size();
}

bool& UReplicationGraphNode_GridSpatialization2D::GetDestroyDormantDynamicActors()
{
    return *(bool*)(GetGridBounds() - TLWCGameLayout<float, double>::Size());
}

TArray<TArray<UReplicationGraphNode_GridCell*>>& UReplicationGraphNode_GridSpatialization2D::GetGrid()
{
    static auto Grid = Anchor::Offset(TEXT("UReplicationGraphNode_GridSpatialization2D::Grid"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rcx, [r14+0x*]; call 0x*")
        .Disp();

    return *(TArray<TArray<UReplicationGraphNode_GridCell*>>*)((uint8*)this + Grid);
}

TArray<UReplicationGraphNode_GridSpatialization2D::FPendingStaticActors>& UReplicationGraphNode_GridSpatialization2D::GetPendingStaticSpatializedActors()
{
    return *(TArray<FPendingStaticActors>*)((uint8*)&GetGrid() - sizeof(TArray<FPendingStaticActors>));
}

TMap<AActor*, UReplicationGraphNode_GridSpatialization2D::FCachedStaticActorInfo>& UReplicationGraphNode_GridSpatialization2D::GetStaticSpatializedActors()
{
    return *(TMap<AActor*, FCachedStaticActorInfo>*)((uint8*)&GetPendingStaticSpatializedActors() - sizeof(TMap<AActor*, FCachedStaticActorInfo>));
}

TMap<AActor*, UReplicationGraphNode_GridSpatialization2D::FCachedDynamicActorInfo>& UReplicationGraphNode_GridSpatialization2D::GetDynamicSpatializedActors()
{
    return *(TMap<AActor*, FCachedDynamicActorInfo>*)((uint8*)&GetStaticSpatializedActors() - sizeof(TMap<AActor*, FCachedDynamicActorInfo>));
}

bool& UReplicationGraphNode_GridSpatialization2D::GetNeedsRebuild()
{
    return *(bool*)((uint8*)&GetGrid() + sizeof(TArray<TArray<UReplicationGraphNode_GridCell*>>));
}

bool UReplicationGraphNode_GridSpatialization2D::WillActorLocationGrowSpatialBounds(const FVector& Location)
{
    const FBox GridBounds = TGameLayout<FBox>::Load(GetGridBounds());

    return GridBounds.IsValid ? false : (Location.X < TLWCGameLayout<float, double>::Load(GetSpatialBias()) || Location.Y < TLWCGameLayout<float, double>::Load(GetSpatialBias() + TLWCGameLayout<float, double>::Size()));
}

void UReplicationGraphNode_GridSpatialization2D::HandleActorOutOfSpatialBounds(AActor* Actor, const FVector& Location3D, const bool bStaticActor)
{
    static auto HandleActorOutOfSpatialBounds = Anchor::Function<void (*)(UReplicationGraphNode_GridSpatialization2D*, AActor*, const TGameValue<FVector>&, bool)>(TEXT("UReplicationGraphNode_GridSpatialization2D::HandleActorOutOfSpatialBounds"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*")
        .Target()
        .Has("mov cl, [rbx+0x*]")
        .Or()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("call 0x*", 2)
        .Target()
        .Get();

    HandleActorOutOfSpatialBounds(this, Actor, TGameValue<FVector>(Location3D), bStaticActor);
}

void UReplicationGraphNode_GridSpatialization2D::PutStaticActorIntoCell(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven)
{
    static auto PutStaticActorIntoCell = Anchor::Function<void (*)(UReplicationGraphNode_GridSpatialization2D*, const FNewReplicatedActorInfo&, FGlobalActorReplicationInfo&, bool)>(TEXT("UReplicationGraphNode_GridSpatialization2D::PutStaticActorIntoCell"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    PutStaticActorIntoCell(this, ActorInfo, ActorRepInfo, bDormancyDriven);
}

void UReplicationGraphNode_GridSpatialization2D::AddActorInternal_Static_Implementation(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& ActorRepInfo, bool bDormancyDriven)
{
    static auto AddActorInternal_Static_Implementation = Anchor::VirtualFunction<void (*)(UReplicationGraphNode_GridSpatialization2D*, const FNewReplicatedActorInfo&, FGlobalActorReplicationInfo&, bool)>(TEXT("UReplicationGraphNode_GridSpatialization2D::AddActorInternal_Static_Implementation"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Get();

    AddActorInternal_Static_Implementation(this, ActorInfo, ActorRepInfo, bDormancyDriven);
}

UReplicationGraphNode_GridCell* UReplicationGraphNode_GridSpatialization2D::GetCellNode(UReplicationGraphNode_GridCell*& NodePtr)
{
    if (!NodePtr)
    {
        auto CreateCellNodeOverride = GetCreateCellNodeOverride();
        auto Callable = *(UReplicationGraphNode_GridCell* (**)(void*, UReplicationGraphNode_GridSpatialization2D*&))CreateCellNodeOverride;

        if (Callable)
        {
            auto Storage = CreateCellNodeOverride + 0x10;
            auto OwnedObject = *(uint8**)Storage ? *(uint8**)Storage : Storage + 0x10;
            auto GetAddress = (*(void* (***)(void*))OwnedObject)[1];
            UReplicationGraphNode_GridSpatialization2D* Self = this;

            NodePtr = Callable(GetAddress(OwnedObject), Self);
        }
        else
        {
            NodePtr = CreateChildNode<UReplicationGraphNode_GridCell>();
        }
    }

    return NodePtr;
}

void UReplicationGraphNode_GridSpatialization2D::PrepareForReplication()
{
    DETOUR(UReplicationGraphNode_GridSpatialization2D, PrepareForReplication, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_GridSpatialization2D::StaticClass()));

    static auto NbDestroyedGridsToTriggerGC = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.NbDestroyedGridsToTriggerGC"));

    auto GlobalRepMap = GetGraphGlobals().IsValid() ? GetGraphGlobals()->GlobalActorReplicationInfoMap : nullptr;
    auto& GatheredNodes = GetGatheredNodes();

    for (auto& MapIt : GetDynamicSpatializedActors())
    {
        AActor* DynamicActor = MapIt.Key;
        auto& DynamicActorInfo = (FCachedDynamicActorInfo&)MapIt.Value;
        auto& PreviousCellInfo = DynamicActorInfo.CellInfo;
        auto& ActorInfo = DynamicActorInfo.ActorInfo;

        auto& ActorRepInfo = GlobalRepMap->Get(DynamicActor);
        const FVector Location3D = DynamicActor->GetActorLocation();
        ActorRepInfo.SetWorldLocation(Location3D);

        if (WillActorLocationGrowSpatialBounds(Location3D))
            HandleActorOutOfSpatialBounds(DynamicActor, Location3D, false);

        if (GetNeedsRebuild())
            continue;

        const FActorCellInfo NewCellInfo = GetCellInfoForActor(DynamicActor, Location3D, ActorRepInfo.GetCullDistance());

        if (!PreviousCellInfo.IsValid())
        {
            GetGridNodesForActor(DynamicActor, NewCellInfo, GatheredNodes);

            for (auto Node : GatheredNodes)
                Node->AddDynamicActor(ActorInfo);

            PreviousCellInfo = NewCellInfo;
            continue;
        }

        bool bDirty = false;

        if (NewCellInfo.StartX > PreviousCellInfo.EndX || NewCellInfo.EndX < PreviousCellInfo.StartX || NewCellInfo.StartY > PreviousCellInfo.EndY || NewCellInfo.EndY < PreviousCellInfo.StartY)
        {
            bDirty = true;

            GetGridNodesForActor(DynamicActor, PreviousCellInfo, GatheredNodes);

            for (auto Node : GatheredNodes)
                Node->RemoveDynamicActor(ActorInfo);

            GetGridNodesForActor(DynamicActor, NewCellInfo, GatheredNodes);

            for (auto Node : GatheredNodes)
                Node->AddDynamicActor(ActorInfo);
        }
        else
        {
            if (PreviousCellInfo.StartX < NewCellInfo.StartX)
            {
                bDirty = true;

                for (int32 X = PreviousCellInfo.StartX; X < NewCellInfo.StartX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = PreviousCellInfo.StartY; Y <= PreviousCellInfo.EndY; ++Y)
                    {
                        if (auto Node = GetCell(GridX, Y))
                            Node->RemoveDynamicActor(ActorInfo);
                    }
                }
            }
            else if (PreviousCellInfo.StartX > NewCellInfo.StartX)
            {
                bDirty = true;

                for (int32 X = NewCellInfo.StartX; X < PreviousCellInfo.StartX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = NewCellInfo.StartY; Y <= NewCellInfo.EndY; ++Y)
                        GetCellNode(GetCell(GridX, Y))->AddDynamicActor(ActorInfo);
                }
            }

            if (PreviousCellInfo.EndX < NewCellInfo.EndX)
            {
                bDirty = true;

                for (int32 X = PreviousCellInfo.EndX + 1; X <= NewCellInfo.EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = NewCellInfo.StartY; Y <= NewCellInfo.EndY; ++Y)
                        GetCellNode(GetCell(GridX, Y))->AddDynamicActor(ActorInfo);
                }
            }
            else if (PreviousCellInfo.EndX > NewCellInfo.EndX)
            {
                bDirty = true;

                for (int32 X = NewCellInfo.EndX + 1; X <= PreviousCellInfo.EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = PreviousCellInfo.StartY; Y <= PreviousCellInfo.EndY; ++Y)
                    {
                        if (auto Node = GetCell(GridX, Y))
                            Node->RemoveDynamicActor(ActorInfo);
                    }
                }
            }

            const int32 StartX = FMath::Max<int32>(NewCellInfo.StartX, PreviousCellInfo.StartX);
            const int32 EndX = FMath::Min<int32>(NewCellInfo.EndX, PreviousCellInfo.EndX);

            if (PreviousCellInfo.StartY < NewCellInfo.StartY)
            {
                bDirty = true;

                for (int32 X = StartX; X <= EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = PreviousCellInfo.StartY; Y < NewCellInfo.StartY; ++Y)
                    {
                        if (auto Node = GetCell(GridX, Y))
                            Node->RemoveDynamicActor(ActorInfo);
                    }
                }
            }
            else if (PreviousCellInfo.StartY > NewCellInfo.StartY)
            {
                bDirty = true;

                for (int32 X = StartX; X <= EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = NewCellInfo.StartY; Y < PreviousCellInfo.StartY; ++Y)
                        GetCellNode(GetCell(GridX, Y))->AddDynamicActor(ActorInfo);
                }
            }

            if (PreviousCellInfo.EndY < NewCellInfo.EndY)
            {
                bDirty = true;

                for (int32 X = StartX; X <= EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = PreviousCellInfo.EndY + 1; Y <= NewCellInfo.EndY; ++Y)
                        GetCellNode(GetCell(GridX, Y))->AddDynamicActor(ActorInfo);
                }
            }
            else if (PreviousCellInfo.EndY > NewCellInfo.EndY)
            {
                bDirty = true;

                for (int32 X = StartX; X <= EndX; ++X)
                {
                    auto& GridX = GetGridX(X);

                    for (int32 Y = NewCellInfo.EndY + 1; Y <= PreviousCellInfo.EndY; ++Y)
                    {
                        if (auto Node = GetCell(GridX, Y))
                            Node->RemoveDynamicActor(ActorInfo);
                    }
                }
            }
        }

        if (bDirty)
            PreviousCellInfo = NewCellInfo;
    }

    auto& PendingStaticSpatializedActors = GetPendingStaticSpatializedActors();

    for (int32 idx = PendingStaticSpatializedActors.Num() - 1; idx >= 0; --idx)
    {
        auto& PendingStaticActor = PendingStaticSpatializedActors[idx];

        if (!PendingStaticActor.Actor->IsActorInitialized())
            continue;

        FNewReplicatedActorInfo NewActorInfo(PendingStaticActor.Actor);
        auto& GlobalInfo = GetGraphGlobals()->GlobalActorReplicationInfoMap->Get(PendingStaticActor.Actor);

        AddActorInternal_Static_Implementation(NewActorInfo, GlobalInfo, PendingStaticActor.DormancyDriven);

        PendingStaticSpatializedActors.RemoveAtSwap(idx, 1, false);
    }

    if (!GetNeedsRebuild())
        return;

    int32 GridsDestroyed = 0;

    for (auto& InnerArray : GetGrid())
    {
        for (auto& N : InnerArray)
        {
            if (N)
            {
                N->TearDown();
                N = nullptr;
                ++GridsDestroyed;
            }
        }
    }

    if (GridsDestroyed >= NbDestroyedGridsToTriggerGC->GetInt())
        GEngine->ForceGarbageCollection(true);

    CleanChildNodes();

    for (auto& MapIt : GetDynamicSpatializedActors())
    {
        AActor* DynamicActor = MapIt.Key;

        if (!IsActorValidForReplicationGather(DynamicActor))
            continue;

        auto& DynamicActorInfo = (FCachedDynamicActorInfo&)MapIt.Value;
        const FVector Location3D = DynamicActor->GetActorLocation();
        auto& ActorRepInfo = GlobalRepMap->Get(DynamicActor);
        ActorRepInfo.SetWorldLocation(Location3D);

        const FActorCellInfo NewCellInfo = GetCellInfoForActor(DynamicActor, Location3D, ActorRepInfo.GetCullDistance());
        GetGridNodesForActor(DynamicActor, NewCellInfo, GatheredNodes);

        for (auto Node : GatheredNodes)
            Node->AddDynamicActor(DynamicActorInfo.ActorInfo);

        DynamicActorInfo.CellInfo = NewCellInfo;
    }

    for (auto& MapIt : GetStaticSpatializedActors())
    {
        AActor* StaticActor = MapIt.Key;

        if (IsActorValidForReplicationGather(StaticActor))
            PutStaticActorIntoCell(MapIt.Value.ActorInfo, GlobalRepMap->Get(StaticActor), MapIt.Value.bDormancyDriven);
    }

    GetNeedsRebuild() = false;
}

TArray<UReplicationGraphNode_GridCell*>& UReplicationGraphNode_GridSpatialization2D::GetGatheredNodes()
{
    static auto GatheredNodes = Anchor::Offset(TEXT("UReplicationGraphNode_GridSpatialization2D::GatheredNodes"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), 0x30)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"))
        .Find("mov r9b, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rbx, [rcx+0x*]")
        .Disp();

    return *(TArray<UReplicationGraphNode_GridCell*>*)((uint8*)this + GatheredNodes);
}

void UReplicationGraphNode_GridSpatialization2D::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_GridSpatialization2D, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_GridSpatialization2D"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_GridSpatialization2D::StaticClass()));

    static auto DormantDynamicActorsDestruction = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.DormantDynamicActorsDestruction"));
    static auto ReplicatedDormantDestructionInfosPerFrame = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.ReplicatedDormantDestructionInfosPerFrame"));

    auto ConnectionManager = Params.GetConnectionManager();
    auto& LastLocationArray = ConnectionManager->GetLastGatherLocations();
    TArray<FIntPoint> UniqueCurrentLocations;
    TArray<FPlayerGridCellInformation> ActiveGridCells;

    const float CellSize = GetCellSize();
    const float SpatialBiasX = TLWCGameLayout<float, double>::Load(GetSpatialBias());
    const float SpatialBiasY = TLWCGameLayout<float, double>::Load(GetSpatialBias() + TLWCGameLayout<float, double>::Size());
    const FBox GridBounds = TGameLayout<FBox>::Load(GetGridBounds());
    const float HalfWorldMax = UE::LWC::IsEnabled() ? 4398046511104.0f : 1048576.0f;

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        auto& CurViewer = Params.GetViewer(Index);
        const FVector ViewLocation = CurViewer.GetViewLocation();

        if (ViewLocation.Z > GetConnectionMaxZ())
            continue;

        const FVector ClampedViewLoc = GridBounds.IsValid ? GridBounds.GetClosestPointTo(ViewLocation) : ViewLocation.BoundToCube(HalfWorldMax);

        int32 CellX = (int32)((ClampedViewLoc.X - SpatialBiasX) / CellSize);

        if (CellX < 0)
            CellX = 0;

        int32 CellY = (int32)((ClampedViewLoc.Y - SpatialBiasY) / CellSize);

        if (CellY < 0)
            CellY = 0;

        FPlayerGridCellInformation NewPlayerCell{ { CellX, CellY }, { 0, 0 } };
        FLastLocationGatherInfo* GatherInfoForConnection = nullptr;

        if (CurViewer.Connection)
        {
            GatherInfoForConnection = LastLocationArray.FindByKey(CurViewer.Connection);

            if (!GatherInfoForConnection)
                GatherInfoForConnection = &LastLocationArray.Emplace(CurViewer.Connection, FVector(ForceInitToZero));
        }

        FVector LastLocationForConnection = GatherInfoForConnection ? GatherInfoForConnection->GetLastLocation() : ClampedViewLoc;
        LastLocationForConnection = GridBounds.IsValid ? GridBounds.GetClosestPointTo(LastLocationForConnection) : LastLocationForConnection.BoundToCube(HalfWorldMax);

        NewPlayerCell.PrevLocation.X = FMath::Max(0, (int32)((LastLocationForConnection.X - SpatialBiasX) / CellSize));
        NewPlayerCell.PrevLocation.Y = FMath::Max(0, (int32)((LastLocationForConnection.Y - SpatialBiasY) / CellSize));

        if (!UniqueCurrentLocations.Contains(NewPlayerCell.CurLocation))
        {
            auto& GridX = GetGridX(CellX);

            if (GridX.Num() <= CellY)
                GridX.SetNum(CellY + 1);

            if (auto CellNode = GridX[CellY])
                CellNode->GatherActorListsForConnection(Params);

            UniqueCurrentLocations.Add(NewPlayerCell.CurLocation);
        }

        ActiveGridCells.Add(NewPlayerCell);
    }

    if (!GetDestroyDormantDynamicActors() || DormantDynamicActorsDestruction->GetInt() <= 0)
        return;

    auto& PrevDormantActorList = ConnectionManager->GetPrevDormantActorListForNode(this);

    for (const auto& CellInfo : ActiveGridCells)
    {
        const int32 CellX = CellInfo.CurLocation.X;
        const int32 CellY = CellInfo.CurLocation.Y;
        const int32 PrevX = CellInfo.PrevLocation.X;
        const int32 PrevY = CellInfo.PrevLocation.Y;

        if ((CellX != PrevX || CellY != PrevY) && !UniqueCurrentLocations.Contains(CellInfo.PrevLocation))
        {
            FActorRepListRefView DormantActorList;
            auto& GridX = GetGridX(CellX);

            if (auto CellNode = GridX[CellY])
            {
                if (auto DormancyNode = CellNode->GetDormancyNode())
                    DormancyNode->ConditionalGatherDormantDynamicActors(DormantActorList, Params, nullptr, false, &PrevDormantActorList);
            }

            if (auto PrevCell = GetCell(GetGridX(PrevX), PrevY))
            {
                if (auto DormancyNode = PrevCell->GetDormancyNode())
                    DormancyNode->ConditionalGatherDormantDynamicActors(PrevDormantActorList, Params, &DormantActorList, true);
            }
        }
    }

    if (PrevDormantActorList.RepList.Num() <= 0)
        return;

    int32 NumActorsToRemove = ReplicatedDormantDestructionInfosPerFrame->GetInt();
    auto GlobalRepMap = GetGraphGlobals().IsValid() ? GetGraphGlobals()->GlobalActorReplicationInfoMap : nullptr;

    for (int32 i = 0; i < PrevDormantActorList.RepList.Num() && NumActorsToRemove > 0; i++)
    {
        AActor* Actor = PrevDormantActorList.RepList[i];
        auto GlobalActorInfo = GlobalRepMap ? GlobalRepMap->Find(Actor) : nullptr;
        auto ActorInfoPtr = ConnectionManager->GetActorInfoMap().ActorMap.Find(Actor);
        auto ActorInfo = ActorInfoPtr ? ActorInfoPtr->Get() : nullptr;

        if (!ActorInfo)
            continue;

        if (ActorInfo->bDormantOnConnection)
        {
            ConnectionManager->NotifyAddDormantDestructionInfo(Actor);
            ActorInfo->bDormantOnConnection = false;
            ActorInfo->bGridSpatilization_AlreadyDormant = false;

            const FVector ActorLocation = GlobalActorInfo ? GlobalActorInfo->GetWorldLocation() : Actor->GetActorLocation();
            const FActorCellInfo ActorCellInfo = GetCellInfoForActor(Actor, ActorLocation, ActorInfo->CullDistance);
            auto& GatheredNodes = GetGatheredNodes();
            GetGridNodesForActor(Actor, ActorCellInfo, GatheredNodes);

            for (auto Node : GatheredNodes)
            {
                if (auto DormancyNode = Node->GetDormancyNode())
                {
                    if (auto ConnectionDormancyNode = DormancyNode->GetExistingConnectionNode(Params))
                        ConnectionDormancyNode->NotifyActorDormancyFlush(Actor);
                }
            }

            NumActorsToRemove--;
            PrevDormantActorList.RepList.RemoveAtSwap(i--);
        }
        else if (!ActorInfo->Channel)
        {
            ActorInfo->bGridSpatilization_AlreadyDormant = false;
            PrevDormantActorList.RepList.RemoveAtSwap(i--);
        }
    }
}

UClass* UReplicationGraphNode_ActorListFrequencyBuckets::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_ActorListFrequencyBuckets"));

    return Class;
}

const UReplicationGraphNode_ActorListFrequencyBuckets::FSettings& UReplicationGraphNode_ActorListFrequencyBuckets::GetSettings()
{
    static auto Settings = Anchor::Offset(TEXT("UReplicationGraphNode_ActorListFrequencyBuckets::Settings"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorListFrequencyBuckets"))
        .Find("inc *")
        .FindEither("call 0x*", "jmp 0x*")
        .Target()
        .Find("cmp qword ptr [rcx+0x*], 0x0")
        .Disp();

    static auto& DefaultSettings = *Anchor::Data<FSettings*>(TEXT("UReplicationGraphNode_ActorListFrequencyBuckets::DefaultSettings"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorListFrequencyBuckets"))
        .Find("inc *")
        .FindEither("call 0x*", "jmp 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Get();

    auto& SettingsPtr = *(TSharedPtr<FSettings>*)((uint8*)this + Settings);

    return SettingsPtr.IsValid() ? *SettingsPtr.Get() : DefaultSettings;
}

TArray<FActorRepListRefView, TInlineAllocator<2>>& UReplicationGraphNode_ActorListFrequencyBuckets::GetNonStreamingCollection()
{
    static auto NonStreamingCollection = Anchor::Offset(TEXT("UReplicationGraphNode_ActorListFrequencyBuckets::NonStreamingCollection"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x28)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorListFrequencyBuckets"))
        .Find("lea rax, [rcx+0x*]")
        .Disp();

    return *(TArray<FActorRepListRefView, TInlineAllocator<2>>*)((uint8*)this + NonStreamingCollection);
}

FStreamingLevelActorListCollection& UReplicationGraphNode_ActorListFrequencyBuckets::GetStreamingLevelCollection()
{
    return *(FStreamingLevelActorListCollection*)((uint8*)&GetNonStreamingCollection() + sizeof(TArray<FActorRepListRefView, TInlineAllocator<2>>));
}

void UReplicationGraphNode_ActorListFrequencyBuckets::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_ActorListFrequencyBuckets, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorListFrequencyBuckets"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_ActorListFrequencyBuckets::StaticClass()));

    const FSettings& MySettings = GetSettings();
    const uint32 ReplicationFrameNum = Params.GetReplicationFrameNum();
    auto& NonStreamingCollection = GetNonStreamingCollection();

    if (MySettings.EnableFastPath)
    {
        const int32 DefaultReplicationIdx = (ReplicationFrameNum % NonStreamingCollection.Num());

        for (int32 idx = 0; idx < NonStreamingCollection.Num(); ++idx)
        {
            if (DefaultReplicationIdx == idx)
            {
                Params.GetOutGatheredReplicationLists()->AddReplicationActorList(NonStreamingCollection[idx], EActorRepListTypeFlags::Default);
            }
            else
            {
                if (ReplicationFrameNum % MySettings.FastPathFrameModulo == 0)
                    Params.GetOutGatheredReplicationLists()->AddReplicationActorList(NonStreamingCollection[idx], EActorRepListTypeFlags::FastShared);
            }
        }
    }
    else
    {
        const int32 idx = ReplicationFrameNum % NonStreamingCollection.Num();
        Params.GetOutGatheredReplicationLists()->AddReplicationActorList(NonStreamingCollection[idx]);
    }

    GetStreamingLevelCollection().Gather(Params);
}

UClass* UReplicationGraphNode_AlwaysRelevant_ForConnection::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_AlwaysRelevant_ForConnection"));

    return Class;
}

FActorRepListRefView& UReplicationGraphNode_AlwaysRelevant_ForConnection::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UReplicationGraphNode_AlwaysRelevant_ForConnection::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x20)
        .Entry(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_AlwaysRelevant_ForConnection"))
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
}

TArray<FAlwaysRelevantActorInfo>& UReplicationGraphNode_AlwaysRelevant_ForConnection::GetPastRelevantActors()
{
    return *(TArray<FAlwaysRelevantActorInfo>*)(&GetReplicationActorList() + 1);
}

void UReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_AlwaysRelevant_ForConnection, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_AlwaysRelevant_ForConnection"), "jmp 0x*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_AlwaysRelevant_ForConnection::StaticClass()));

    Original(Params);

    auto& ReplicationActorList = GetReplicationActorList();
    auto ConnectionManager = Params.GetConnectionManager();

    auto UpdateActor = [&](AActor* NewActor, AActor*& LastActor)
    {
        if (LastActor != NewActor)
        {
            if (NewActor)
                ConnectionManager->GetActorInfoMap().FindOrAdd(NewActor).SetCullDistanceSquared(0.f);

            if (LastActor)
                ConnectionManager->GetActorInfoMap().FindOrAdd(LastActor).SetCullDistanceSquared(GetGraphGlobals()->GlobalActorReplicationInfoMap->Get(LastActor).GetCullDistanceSquared());

            LastActor = NewActor;
        }

        if (NewActor && !ReplicationActorList.RepList.Contains(NewActor))
            ReplicationActorList.RepList.Add(NewActor);
    };

    ReplicationActorList.RepList.Reset();

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        const FNetViewer& CurViewer = Params.GetViewer(Index);

        if (!CurViewer.Connection)
            continue;

        if (ConnectionManager->GetNetConnection()->IsReplay() && CurViewer.Connection != ConnectionManager->GetNetConnection())
            continue;

        auto& PastRelevantActors = GetPastRelevantActors();
        auto LastData = PastRelevantActors.FindByKey(CurViewer.Connection);

        if (!LastData)
        {
            FAlwaysRelevantActorInfo NewActorInfo = { CurViewer.Connection, nullptr, nullptr };
            LastData = &PastRelevantActors[PastRelevantActors.Add(NewActorInfo)];
        }

        UpdateActor(CurViewer.InViewer, LastData->LastViewer);
        UpdateActor(CurViewer.ViewTarget, LastData->LastViewTarget);
    }

    GetPastRelevantActors().RemoveAll([](const FAlwaysRelevantActorInfo& RelActorInfo)
    {
        return RelActorInfo.Connection == nullptr;
    });

    if (ReplicationActorList.RepList.Num() > 0)
        Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ReplicationActorList);
}

UClass* UReplicationGraphNode_TearOff_ForConnection::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraphNode_TearOff_ForConnection"));

    return Class;
}

TArray<UReplicationGraphNode_TearOff_ForConnection::FTearOffActorInfo>& UReplicationGraphNode_TearOff_ForConnection::GetTearOffActors()
{
    static auto TearOffActors = Anchor::Offset(TEXT("UReplicationGraphNode_TearOff_ForConnection::TearOffActors"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x8)
        .Find("lea rcx, [rdi+0x*]; call 0x*")
        .Disp();

    return *(TArray<FTearOffActorInfo>*)((uint8*)this + TearOffActors);
}

FActorRepListRefView& UReplicationGraphNode_TearOff_ForConnection::GetReplicationActorList()
{
    return *(FActorRepListRefView*)((uint8*)&GetTearOffActors() + sizeof(TArray<FTearOffActorInfo>));
}

void UReplicationGraphNode_TearOff_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UReplicationGraphNode_TearOff_ForConnection, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_TearOff_ForConnection"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraphNode_TearOff_ForConnection::StaticClass()));

    auto& TearOffActors = GetTearOffActors();

    if (TearOffActors.Num() > 0)
    {
        auto& ReplicationActorList = GetReplicationActorList();
        ReplicationActorList.RepList.Reset();

        auto& ActorInfoMap = Params.GetConnectionManager()->GetActorInfoMap();

        for (int32 idx = TearOffActors.Num() - 1; idx >= 0; --idx)
        {
            auto& TearOffInfo = TearOffActors[idx];
            AActor* Actor = TearOffInfo.Actor;
            const uint32 TearOffFrameNum = TearOffInfo.TearOffFrameNum;

            if (Actor && IsActorValidForReplication(Actor))
            {
                if (auto ActorInfoPtr = ActorInfoMap.ActorMap.Find(Actor))
                {
                    auto ActorInfo = ActorInfoPtr->Get();

                    if (ActorInfo && ActorInfo->LastRepFrameNum <= TearOffFrameNum && !(ActorInfo->LastRepFrameNum <= 0 && TearOffInfo.bHasReppedOnce))
                    {
                        ReplicationActorList.RepList.Add(Actor);
                        TearOffInfo.bHasReppedOnce = true;
                        continue;
                    }
                }
            }

            TearOffActors.RemoveAtSwap(idx, 1, false);
        }

        if (ReplicationActorList.RepList.Num() > 0)
            Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ReplicationActorList);
    }
}

void UReplicationGraphNode::NotifyResetAllNetworkActors()
{
    static auto NotifyResetAllNetworkActors = Anchor::VirtualFunction<void (*)(UReplicationGraphNode*)>(TEXT("UReplicationGraphNode::NotifyResetAllNetworkActors"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x28)
        .Get();

    NotifyResetAllNetworkActors(this);
}

void UNetReplicationGraphConnection::NotifyResetAllNetworkActors()
{
    static auto NotifyResetAllNetworkActors = Anchor::VirtualFunction<void (*)(UNetReplicationGraphConnection*)>(TEXT("UNetReplicationGraphConnection::NotifyResetAllNetworkActors"))
        .String(TEXT("UNetReplicationGraphConnection_NotifyActorChannelCleanedUp"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__NetReplicationGraphConnection"), 0x40)
        .Get();

    NotifyResetAllNetworkActors(this);
}

void UNetReplicationGraphConnection::InitForGraph(UReplicationGraph* Graph)
{
    if (!Graph)
        return;

    auto& Globals = Graph->GetGraphGlobals();

    if (Globals.IsValid())
        GetGlobalMap() = Globals->GlobalActorReplicationInfoMap;
}

void UNetReplicationGraphConnection::InitForConnection(UNetConnection* InConnection)
{
    GetNetConnection() = InConnection;
    InConnection->GetReplicationConnectionDriver() = this;
}

UClass* UReplicationGraph::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ReplicationGraph.ReplicationGraph"));

    return Class;
}

UClass* UReplicationGraph::GetReplicationConnectionManagerClass()
{
    static auto ReplicationConnectionManagerClass = StaticClass()->FindPropertyByName(FName(TEXT("ReplicationConnectionManagerClass")));

    return *ReplicationConnectionManagerClass->ContainerPtrToValuePtr<UClass*>(this);
}

UNetDriver*& UReplicationGraph::GetNetDriver()
{
    static auto NetDriver = StaticClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    return *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

TArray<UNetReplicationGraphConnection*>& UReplicationGraph::GetConnections()
{
    static auto Connections = StaticClass()->FindPropertyByName(FName(TEXT("Connections")));

    return *Connections->ContainerPtrToValuePtr<TArray<UNetReplicationGraphConnection*>>(this);
}

TArray<UNetReplicationGraphConnection*>& UReplicationGraph::GetPendingConnectionManagers()
{
    static auto PendingConnections = StaticClass()->FindPropertyByName(FName(TEXT("PendingConnections")));

    return *PendingConnections->ContainerPtrToValuePtr<TArray<UNetReplicationGraphConnection*>>(this);
}

TArray<UReplicationGraphNode*>& UReplicationGraph::GetPrepareForReplicationNodes()
{
    static auto PrepareForReplicationNodes = StaticClass()->FindPropertyByName(FName(TEXT("PrepareForReplicationNodes")));

    return *PrepareForReplicationNodes->ContainerPtrToValuePtr<TArray<UReplicationGraphNode*>>(this);
}

TArray<UReplicationGraphNode*>& UReplicationGraph::GetGlobalGraphNodes()
{
    static auto GlobalGraphNodes = StaticClass()->FindPropertyByName(FName(TEXT("GlobalGraphNodes")));

    return *GlobalGraphNodes->ContainerPtrToValuePtr<TArray<UReplicationGraphNode*>>(this);
}

TSharedPtr<FReplicationGraphGlobalData>& UReplicationGraph::GetGraphGlobals()
{
    static auto GraphGlobals = Anchor::Offset(TEXT("UReplicationGraph::GraphGlobals"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x88)
        .Find("mov rax, [rcx+0x*]")
        .Disp();

    return *(TSharedPtr<FReplicationGraphGlobalData>*)((uint8*)this + GraphGlobals);
}

float& UReplicationGraph::GetDestructInfoMaxDistanceSquared()
{
    return *(float*)(&GetGraphGlobals() + 1);
}

UReplicationGraph::FPrioritizationConstants& UReplicationGraph::GetPrioritizationConstants()
{
    return *(FPrioritizationConstants*)(&GetDestructInfoMaxDistanceSquared() + 1);
}

UReplicationGraph::FFastSharedPathConstants& UReplicationGraph::GetFastSharedPathConstants()
{
    return *(FFastSharedPathConstants*)(&GetPrioritizationConstants() + 1);
}

uint32& UReplicationGraph::GetGlobalActorChannelFrameNumTimeout()
{
    return *(uint32*)(&GetFastSharedPathConstants() + 1);
}

FPrioritizedRepList& UReplicationGraph::GetPrioritizedReplicationList()
{
    return *(FPrioritizedRepList*)(&GetGlobalGraphNodes() - 1);
}

FFrameReplicationStats& UReplicationGraph::GetFrameReplicationStats()
{
    static auto FrameReplicationStats = Anchor::Offset(TEXT("UReplicationGraph::FrameReplicationStats"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x38)
        .Find("movdqu [rcx+0x*], xmm0")
        .Disp();

    return *(FFrameReplicationStats*)((uint8*)this + FrameReplicationStats);
}

bool& UReplicationGraph::GetWasConnectionSaturated()
{
    return *(bool*)(&GetFrameReplicationStats() + 1);
}

uint32& UReplicationGraph::GetReplicationGraphFrame()
{
    return *(uint32*)((uint8*)&GetWasConnectionSaturated() + sizeof(uint32));
}

int32& UReplicationGraph::GetActorDiscoveryMaxBitsPerFrame()
{
    return *(int32*)(&GetReplicationGraphFrame() + 1);
}

TArray<UNetConnection*>& UReplicationGraph::GetConnectionsNeedingsPostTickDispatchFlush()
{
    static auto ConnectionsNeedingsPostTickDispatchFlush = Anchor::Offset(TEXT("UReplicationGraph::ConnectionsNeedingsPostTickDispatchFlush"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x40)
        .Find("lea rdi, [rcx+0x*]")
        .Disp();

    return *(TArray<UNetConnection*>*)((uint8*)this + ConnectionsNeedingsPostTickDispatchFlush);
}

UActorChannel*& UReplicationGraph::GetFastSharedReplicationChannel()
{
    return *((UActorChannel**)&GetConnectionsNeedingsPostTickDispatchFlush() - 2);
}

FOutBunch*& UReplicationGraph::GetFastSharedReplicationBunch()
{
    return *((FOutBunch**)&GetFastSharedReplicationChannel() - 1);
}

FName UReplicationGraph::GetFastSharedReplicationFuncName()
{
    return TGameLayout<FName>::Load(&GetFastSharedReplicationChannel() + 1);
}

void UReplicationGraph::SetFastSharedReplicationFuncName(const FName& InFastSharedReplicationFuncName)
{
    TGameLayout<FName>::Store(&GetFastSharedReplicationChannel() + 1, InFastSharedReplicationFuncName);
}

int32 UReplicationGraph::ServerReplicateActors(float DeltaSeconds)
{
    DETOUR(UReplicationGraph, ServerReplicateActors, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x38)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "push rbp; lea rbp, [rsp-0x*]; sub rsp, 0x*; mov rax, [0x*]; xor rax, rsp")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    static auto OutOfRangeDistanceCheckRatio = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.OutOfRangeDistanceCheckRatio"));

    UNetDriver* NetDriver = GetNetDriver();
    NetDriver->SetReplicationFrame(NetDriver->GetReplicationFrame() + 1);

    const uint32 FrameNum = GetReplicationGraphFrame();

    GetFrameReplicationStats() = FFrameReplicationStats{};
    GetWasConnectionSaturated() = false;

    TSet<UNetConnection*> ConnectionsToClose;

    auto OnScopeExit = [&]()
    {
        GetReplicationGraphFrame()++;

        for (UNetConnection* ConnectionToClose : ConnectionsToClose)
            ConnectionToClose->Close();
    };

    for (UReplicationGraphNode* Node : GetPrepareForReplicationNodes())
        Node->PrepareForReplication();

    int32 NumChildrenConnectionsProcessed = 0;

    for (UNetReplicationGraphConnection* ConnectionManager : GetConnections())
    {
        if (ConnectionManager->PrepareForReplication() == false)
            continue;

        FNetViewerArray ConnectionViewers;
        UNetConnection* const NetConnection = ConnectionManager->GetNetConnection();
        APlayerController* const PC = NetConnection->GetPlayerController();
        FPerConnectionActorInfoMap& ConnectionActorInfoMap = ConnectionManager->GetActorInfoMap();

        const bool bReplayConnection = NetConnection->IsReplay();

        if (bReplayConnection && !NetConnection->IsReplayReady())
            continue;

        ConnectionViewers.Emplace(NetConnection, 0.f);

        if (PC)
            PC->SendClientAdjustment();

        auto& Children = NetConnection->GetChildren();

        for (int32 ChildIdx = 0; ChildIdx < Children.Num(); ++ChildIdx)
        {
            UNetConnection* ChildConnection = Children[ChildIdx];

            if (ChildConnection && ChildConnection->GetPlayerController() && ChildConnection->GetViewTarget())
            {
                ChildConnection->GetPlayerController()->SendClientAdjustment();
                ConnectionViewers.Emplace(ChildConnection, 0.f);
            }
        }

        NumChildrenConnectionsProcessed += Children.Num();

        if (bReplayConnection)
        {
            for (UNetReplicationGraphConnection* RepGraphConn : GetConnections())
            {
                if ((RepGraphConn->GetNetConnection() != NetConnection) && !RepGraphConn->GetNetConnection()->IsReplay() && RepGraphConn->PrepareForReplication())
                    ConnectionViewers.Emplace(RepGraphConn->GetNetConnection(), 0.f);
            }

            AddReplayViewers(NetConnection, ConnectionViewers);
        }

        auto OnConnectionScopeExit = [&]()
        {
            NetConnection->SetTickCount(NetConnection->GetTickCount() + 1);
            GetWasConnectionSaturated() = false;
        };

        const FReplicationGraphDestructionSettings DestructionSettings(GetDestructInfoMaxDistanceSquared(), OutOfRangeDistanceCheckRatio->GetFloat() * GetDestructInfoMaxDistanceSquared());

        ConnectionManager->GetQueuedBitsForActorDiscovery() = 0;

        FGatheredReplicationActorLists GatheredReplicationListsForConnection{};
        TSet<FName> AllVisibleLevelNames;

        ConnectionManager->GetClientVisibleLevelNames(AllVisibleLevelNames);

        const FConnectionGatherActorListParameters Parameters(ConnectionViewers, *ConnectionManager, AllVisibleLevelNames, FrameNum, GatheredReplicationListsForConnection);

        for (UReplicationGraphNode* Node : GetGlobalGraphNodes())
            Node->GatherActorListsForConnection(Parameters);

        for (UReplicationGraphNode* Node : ConnectionManager->GetConnectionGraphNodes())
            Node->GatherActorListsForConnection(Parameters);

        ConnectionManager->UpdateGatherLocationsForConnection(ConnectionViewers, DestructionSettings);

        if (GatheredReplicationListsForConnection.CachedNum == 0)
        {
            OnConnectionScopeExit();
            OnScopeExit();

            return 0;
        }

        ReplicateActorListsForConnections_Default(ConnectionManager, GatheredReplicationListsForConnection, ConnectionViewers);
        ReplicateActorListsForConnections_FastShared(ConnectionManager, GatheredReplicationListsForConnection, ConnectionViewers);

        for (const auto& Pair : ConnectionActorInfoMap.ChannelMap)
        {
            UActorChannel* Channel = Pair.Key;
            FConnectionReplicationActorInfo& ConnectionActorInfo = *Pair.Value.Get();

            if (Channel->GetClosing())
                continue;
            else if (ConnectionActorInfo.Channel->GetClosing())
                continue;

            if (ConnectionActorInfo.ActorChannelCloseFrameNum > 0 && ConnectionActorInfo.ActorChannelCloseFrameNum <= FrameNum)
            {
                AActor* Actor = Channel->GetActor();

                if (Actor)
                {
                    if (Actor->IsNetStartupActor())
                        continue;

                    ConnectionActorInfo.Channel->Close(EChannelCloseReason::Relevancy);
                    ConnectionManager->SetActorNotDormantOnConnection(Actor);
                }
            }
        }

        ConnectionManager->ReplicateDestructionInfos(ConnectionViewers, DestructionSettings.DestructInfoMaxDistanceSquared);
        ConnectionManager->ReplicateDormantDestructionInfos();

        if (NetConnection->GetPendingCloseDueToReplicationFailure())
            ConnectionsToClose.Add(NetConnection);

        OnConnectionScopeExit();
    }

    GetFrameReplicationStats().NumConnections = GetConnections().Num() + NumChildrenConnectionsProcessed;
    PostServerReplicateStats(GetFrameReplicationStats());

    OnScopeExit();

    return 0;
}

void UReplicationGraph::AddReplayViewers(UNetConnection* NetConnection, FNetViewerArray& Viewers)
{
    static auto AddReplayViewers = Anchor::VirtualFunction<void (*)(UReplicationGraph*, UNetConnection*, FNetViewerArray&)>(TEXT("UReplicationGraph::AddReplayViewers"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0xA8)
        .Get();

    AddReplayViewers(this, NetConnection, Viewers);
}

void UReplicationGraph::PostServerReplicateStats(const FFrameReplicationStats& Stats)
{
    static auto PostServerReplicateStats = Anchor::VirtualFunction<void (*)(UReplicationGraph*, const FFrameReplicationStats&)>(TEXT("UReplicationGraph::PostServerReplicateStats"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0xB0)
        .Get();

    PostServerReplicateStats(this, Stats);
}

void UReplicationGraph::ReplicateActorListsForConnections_Default(UNetReplicationGraphConnection* ConnectionManager, FGatheredReplicationActorLists& GatheredReplicationListsForConnection, FNetViewerArray& Viewers)
{
    static auto ForceConnectionViewerPriority = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.ForceConnectionViewerPriority"));

    auto NetConnection = ConnectionManager->GetNetConnection();
    auto& ConnectionActorInfoMap = ConnectionManager->GetActorInfoMap();
    const uint32 FrameNum = GetReplicationGraphFrame();
    auto& PrioritizedReplicationList = GetPrioritizedReplicationList();
    const auto& PrioritizationConstants = GetPrioritizationConstants();

    PrioritizedReplicationList.Items.Reset();

    for (auto List : GatheredReplicationListsForConnection.ReplicationLists[(uint32)EActorRepListTypeFlags::Default])
    {
        for (auto Actor : List->RepList)
        {
            FConnectionReplicationActorInfo& ConnectionData = ConnectionActorInfoMap.FindOrAdd(Actor);

            if (ConnectionData.bDormantOnConnection)
                continue;

            FGlobalActorReplicationInfo& GlobalData = GetGlobalActorReplicationInfoMap().Get(Actor);

            if (!(ConnectionData.NextReplicationFrameNum <= FrameNum || GlobalData.GetForceNetUpdateFrame() > ConnectionData.LastRepFrameNum))
                continue;

            float AccumulatedPriority = GlobalData.GetAccumulatedNetPriorityBias();

            if (GlobalData.GetDistancePriorityScale() > 0.f)
            {
                float SmallestDistanceSq = FLT_MAX;
                int32 ViewersThatSkipActor = 0;
                const FVector WorldLocation = GlobalData.GetWorldLocation();

                for (const FNetViewer& CurViewer : Viewers)
                {
                    const FVector ViewLocation = CurViewer.GetViewLocation();
                    const float DistSq = (WorldLocation.Y - ViewLocation.Y) * (WorldLocation.Y - ViewLocation.Y) + (WorldLocation.X - ViewLocation.X) * (WorldLocation.X - ViewLocation.X) + (WorldLocation.Z - ViewLocation.Z) * (WorldLocation.Z - ViewLocation.Z);

                    SmallestDistanceSq = DistSq < SmallestDistanceSq ? DistSq : SmallestDistanceSq;

                    if (ConnectionData.CullDistanceSquared > 0.f && DistSq > ConnectionData.CullDistanceSquared)
                        ++ViewersThatSkipActor;
                }

                if (ViewersThatSkipActor >= Viewers.Num())
                    continue;

                const float DistanceFactor = FMath::Clamp(SmallestDistanceSq / PrioritizationConstants.MaxDistanceScaling, 0.f, 1.f) * GlobalData.GetDistancePriorityScale();
                AccumulatedPriority += DistanceFactor;
            }

            UpdateActorChannelCloseFrameNum(Actor, ConnectionData, GlobalData, FrameNum, NetConnection);

            if (GlobalData.GetStarvationPriorityScale() > 0.f)
            {
                const uint32 FramesSinceLastRep = FrameNum - ConnectionData.LastRepFrameNum;
                const float StarvationFactor = 1.f - FMath::Clamp(((float)FramesSinceLastRep * GlobalData.GetStarvationPriorityScale()) / (float)PrioritizationConstants.MaxFramesSinceLastRep, 0.f, 1.f);

                AccumulatedPriority += StarvationFactor;
            }

            if (GlobalData.GetWantsToBeDormant() && ConnectionData.LastRepFrameNum > 0)
                AccumulatedPriority += -1.5f;

            if (GlobalData.GetForceNetUpdateFrame() > ConnectionData.LastRepFrameNum)
                AccumulatedPriority += -1.f;

            for (const FNetViewer& CurViewer : Viewers)
            {
                if (Actor == CurViewer.ViewTarget || Actor == CurViewer.InViewer)
                {
                    if (ForceConnectionViewerPriority->GetFloat() > 0.f)
                        AccumulatedPriority = -FLT_MAX;
                    else
                        AccumulatedPriority += -10.f;

                    break;
                }
            }

            PrioritizedReplicationList.Items.Add(FPrioritizedRepList::FItem(AccumulatedPriority, Actor, &GlobalData, &ConnectionData));
        }
    }

    AlgoImpl::IntroSortInternal(PrioritizedReplicationList.Items.GetData(), PrioritizedReplicationList.Items.Num(), [](const FPrioritizedRepList::FItem& A, const FPrioritizedRepList::FItem& B)
    {
        return A < B;
    });

    ReplicateActorsForConnection(NetConnection, ConnectionActorInfoMap, ConnectionManager, FrameNum);

    ConnectionManager->GetOnPostReplicatePrioritizeLists().Broadcast(ConnectionManager, &PrioritizedReplicationList);
}

void UReplicationGraph::ReplicateActorsForConnection(UNetConnection* NetConnection, FPerConnectionActorInfoMap& ConnectionActorInfoMap, UNetReplicationGraphConnection* ConnectionManager, const uint32 FrameNum)
{
    auto& PrioritizedReplicationList = GetPrioritizedReplicationList();

    for (int32 ActorIdx = 0; ActorIdx < PrioritizedReplicationList.Items.Num(); ++ActorIdx)
    {
        const FPrioritizedRepList::FItem& RepItem = PrioritizedReplicationList.Items[ActorIdx];

        AActor* Actor = RepItem.Actor;
        FConnectionReplicationActorInfo& ActorInfo = *RepItem.ConnectionData;

        if (ActorInfo.LastRepFrameNum == FrameNum)
            continue;

        FGlobalActorReplicationInfo& GlobalActorInfo = *RepItem.GlobalData;

        ReplicateSingleActor(Actor, ActorInfo, GlobalActorInfo, ConnectionActorInfoMap, *ConnectionManager, FrameNum);

        if (!NetConnection->IsNetReady(false))
        {
            HandleStarvedActorList(PrioritizedReplicationList, ActorIdx + 1, ConnectionActorInfoMap, FrameNum);
            NotifyConnectionSaturated(*ConnectionManager);
            break;
        }
    }
}

void UReplicationGraph::ReplicateActorListsForConnections_FastShared(UNetReplicationGraphConnection* ConnectionManager, FGatheredReplicationActorLists& GatheredReplicationListsForConnection, FNetViewerArray& Viewers)
{
    const auto& GatheredLists = GatheredReplicationListsForConnection.ReplicationLists[(uint32)EActorRepListTypeFlags::FastShared];

    if (GatheredLists.Num() <= 0)
        return;

    auto& ConnectionActorInfoMap = ConnectionManager->GetActorInfoMap();
    UNetConnection* const NetConnection = ConnectionManager->GetNetConnection();
    const uint32 FrameNum = GetReplicationGraphFrame();
    const auto& FastSharedPathConstants = GetFastSharedPathConstants();
    const float FastSharedDistanceRequirementPct = FastSharedPathConstants.DistanceRequirementPct;
    const int64 MaxBits = FastSharedPathConstants.MaxBitsPerFrame;
    const int32 StartIdx = FrameNum * FastSharedPathConstants.ListSkipPerFrame;

    int32 TotalBitsWritten = 0;

    for (int32 ListIdx = 0; ListIdx < GatheredLists.Num(); ++ListIdx)
    {
        const auto& List = GatheredLists[(ListIdx + FrameNum) % GatheredLists.Num()]->RepList;

        for (int32 i = 0; i < List.Num(); ++i)
        {
            AActor* Actor = List[(i + StartIdx) % List.Num()];

            FConnectionReplicationActorInfo& ConnectionData = ConnectionActorInfoMap.FindOrAdd(Actor);

            if (ConnectionData.LastRepFrameNum == FrameNum)
                continue;

            if (ConnectionData.bTearOff)
                continue;

            UActorChannel* ActorChannel = ConnectionData.Channel;

            if (ActorChannel == nullptr || ActorChannel->GetClosing())
                continue;

            FGlobalActorReplicationInfo& GlobalActorInfo = GetGlobalActorReplicationInfoMap().Get(Actor);

            if (*(void**)GlobalActorInfo.GetFastSharedReplicationFunc() == nullptr)
                continue;

            bool bNoViewRelevency = true;
            const FVector WorldLocation = GlobalActorInfo.GetWorldLocation();

            for (const FNetViewer& CurView : Viewers)
            {
                const FVector DirToActor = WorldLocation - CurView.GetViewLocation();

                if (!((DirToActor | CurView.GetViewDir()) < 0.f))
                {
                    bNoViewRelevency = false;
                    break;
                }

                const float DistSq = DirToActor.SizeSquared();

                if (!(DistSq > (ConnectionData.CullDistanceSquared * FastSharedDistanceRequirementPct)))
                {
                    bNoViewRelevency = false;
                    break;
                }
            }

            if (bNoViewRelevency)
                continue;

            TotalBitsWritten += (int32)ReplicateSingleActor_FastShared(Actor, ConnectionData, GlobalActorInfo, *ConnectionManager, FrameNum);

            if (TotalBitsWritten > MaxBits)
            {
                NotifyConnectionSaturated(*ConnectionManager);
                NetConnection->GetQueuedBits() -= TotalBitsWritten;
                return;
            }
        }
    }

    NetConnection->GetQueuedBits() -= TotalBitsWritten;
}

int64 UReplicationGraph::ReplicateSingleActor_FastShared(AActor* Actor, FConnectionReplicationActorInfo& ConnectionData, FGlobalActorReplicationInfo& GlobalActorInfo, UNetReplicationGraphConnection& ConnectionManager, const uint32 FrameNum)
{
    UNetConnection* NetConnection = ConnectionManager.GetNetConnection();

    const int64 BitsWritten = [&]() -> int64
    {
        FScopedActorRoleSwap SwapGuard(GlobalActorInfo.GetSwapRolesOnReplicate() ? Actor : nullptr);

        if (NetConnection->IsReplay())
            return ReplicateSingleActor(Actor, ConnectionData, GlobalActorInfo, ConnectionManager.GetActorInfoMap(), ConnectionManager, FrameNum);

        UActorChannel* ActorChannel = ConnectionData.Channel;

        if (ActorChannel == nullptr || ActorChannel->GetClosing())
            return 0;

        GetFrameReplicationStats().NumReplicatedFastPathActors++;

        auto& FastSharedReplicationInfo = GlobalActorInfo.GetFastSharedReplicationInfo();

        if (!FastSharedReplicationInfo)
        {
            FastSharedReplicationInfo = (FFastSharedReplicationInfo*)FMemory::Realloc(nullptr, sizeof(FFastSharedReplicationInfo) + FOutBunch::GetTypeSize());
            FastSharedReplicationInfo->LastAttemptBuildFrameNum = 0;
            FastSharedReplicationInfo->LastBunchBuildFrameNum = 0;
            FMemory::Memzero(&FastSharedReplicationInfo->GetBunch(), FOutBunch::GetTypeSize());
            ::new(&FastSharedReplicationInfo->GetBunch()) FOutBunch();
        }

        FOutBunch& OutBunch = FastSharedReplicationInfo->GetBunch();

        if (FastSharedReplicationInfo->LastAttemptBuildFrameNum < FrameNum)
        {
            FastSharedReplicationInfo->LastAttemptBuildFrameNum = FrameNum;

            auto FastSharedReplicationFunc = GlobalActorInfo.GetFastSharedReplicationFunc();
            auto Callable = *(bool (**)(void*, AActor*&))FastSharedReplicationFunc;

            if (Callable == nullptr)
                return 0;

            GetFastSharedReplicationBunch() = &OutBunch;
            GetFastSharedReplicationChannel() = ActorChannel;
            SetFastSharedReplicationFuncName(GlobalActorInfo.GetFastSharedReplicationFuncName());

            auto Storage = FastSharedReplicationFunc + 0x10;
            auto OwnedObject = *(uint8**)Storage ? *(uint8**)Storage : Storage + 0x10;
            auto GetAddress = (*(void* (***)(void*))OwnedObject)[1];

            if (Callable(GetAddress(OwnedObject), Actor) == false)
            {
                GetFastSharedReplicationBunch() = nullptr;
                GetFastSharedReplicationChannel() = nullptr;
                SetFastSharedReplicationFuncName(FName(NAME_None));
                return 0;
            }

            if (GetFastSharedReplicationBunch() == nullptr)
            {
                FastSharedReplicationInfo->LastBunchBuildFrameNum = FrameNum;
            }
            else
            {
                GetFastSharedReplicationBunch() = nullptr;
                GetFastSharedReplicationChannel() = nullptr;
                SetFastSharedReplicationFuncName(FName(NAME_None));
            }
        }

        if (ConnectionData.FastPath_LastRepFrameNum >= FastSharedReplicationInfo->LastBunchBuildFrameNum)
            return 0;

        if (OutBunch.GetNumBits() <= 0)
            return 0;

        OutBunch.SetChName(ActorChannel->GetChName());
        OutBunch.SetChIndex(ActorChannel->GetChIndex());
        OutBunch.SetChannel(ActorChannel);
        OutBunch.SetNext(nullptr);

        auto& bHoldQueuedExportBunchesAndGUIDs = ActorChannel->GetHoldQueuedExportBunchesAndGUIDs();
        const bool bOldHoldQueuedExportBunchesAndGUIDs = bHoldQueuedExportBunchesAndGUIDs;

        bHoldQueuedExportBunchesAndGUIDs = true;
        ActorChannel->SendBunch(&OutBunch, false);
        bHoldQueuedExportBunchesAndGUIDs = bOldHoldQueuedExportBunchesAndGUIDs;

        return OutBunch.GetNumBits();
    }();

    ConnectionData.FastPath_LastRepFrameNum = FrameNum;
    ConnectionData.FastPath_NextReplicationFrameNum = FrameNum + ConnectionData.FastPath_ReplicationPeriodFrame;

    return BitsWritten;
}

int64 UReplicationGraph::ReplicateSingleActor(AActor* Actor, FConnectionReplicationActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalActorInfo, FPerConnectionActorInfoMap& ConnectionActorInfoMap, UNetReplicationGraphConnection& ConnectionManager, const uint32 FrameNum)
{
    if (!Actor)
        return 0;

    UNetConnection* NetConnection = ConnectionManager.GetNetConnection();

    if (!IsActorValidForReplication(Actor))
        return 0;

    if (ActorInfo.Channel)
    {
        if (ActorInfo.Channel->GetClosing())
            return 0;
        else if (ActorInfo.Channel->GetActor() != Actor)
            return 0;
    }

    GetFrameReplicationStats().NumReplicatedActors++;
    ActorInfo.LastRepFrameNum = FrameNum;
    ActorInfo.NextReplicationFrameNum = FrameNum + ActorInfo.ReplicationPeriodFrame;

    if (GlobalActorInfo.GetLastPreReplicationFrame() != FrameNum)
    {
        GlobalActorInfo.GetLastPreReplicationFrame() = FrameNum;
        Actor->CallPreReplication(GetNetDriver());
    }

    FScopedActorRoleSwap SwapGuard(GlobalActorInfo.GetSwapRolesOnReplicate() ? Actor : nullptr);

    const bool bWantsToGoDormant = GlobalActorInfo.GetWantsToBeDormant();

    if (ActorInfo.Channel == nullptr)
    {
        ActorInfo.Channel = (UActorChannel*)NetConnection->CreateChannelByName(FName(NAME_Actor), EChannelCreateFlags::OpenedLocally);

        if (!ActorInfo.Channel)
            return 0;

        ActorInfo.Channel->SetChannelActor(Actor, ESetChannelActorFlags::None);
    }

    if (bWantsToGoDormant)
        ActorInfo.Channel->StartBecomingDormant();

    int64 BitsWritten = 0;

    if (ActorInfo.bTearOff)
    {
        BitsWritten = ActorInfo.Channel->ReplicateActor();
        BitsWritten += ActorInfo.Channel->Close(EChannelCloseReason::TearOff);
    }
    else
    {
        BitsWritten = ActorInfo.Channel->ReplicateActor();
    }

    if (!(BitsWritten > 0))
        GetFrameReplicationStats().NumReplicatedCleanActors++;

    const bool bIsTrafficActorDiscovery = GetActorDiscoveryMaxBitsPerFrame() > 0 && (ActorInfo.Channel && ActorInfo.Channel->GetSpawnAcked() == false);
    const bool bIsActorDiscoveryBudgetFull = bIsTrafficActorDiscovery && (ConnectionManager.GetQueuedBitsForActorDiscovery() >= GetActorDiscoveryMaxBitsPerFrame());

    auto& DependentActorList = GlobalActorInfo.GetDependentActorList();

    if (DependentActorList.Num() > 0)
    {
        const uint32 CloseFrameNum = ActorInfo.ActorChannelCloseFrameNum;

        for (AActor* DependentActor : DependentActorList)
        {
            FConnectionReplicationActorInfo& DependentActorConnectionInfo = ConnectionActorInfoMap.FindOrAdd(DependentActor);
            FGlobalActorReplicationInfo& DependentActorGlobalData = GetGlobalActorReplicationInfoMap().Get(DependentActor);

            UpdateActorChannelCloseFrameNum(DependentActor, DependentActorConnectionInfo, DependentActorGlobalData, FrameNum, NetConnection);

            DependentActorConnectionInfo.ActorChannelCloseFrameNum = CloseFrameNum > DependentActorConnectionInfo.ActorChannelCloseFrameNum ? CloseFrameNum : DependentActorConnectionInfo.ActorChannelCloseFrameNum;

            if (!(DependentActorConnectionInfo.NextReplicationFrameNum <= FrameNum || DependentActorGlobalData.GetForceNetUpdateFrame() > DependentActorConnectionInfo.LastRepFrameNum))
                continue;

            if (!IsActorValidForReplication(DependentActor))
                continue;

            BitsWritten += ReplicateSingleActor(DependentActor, DependentActorConnectionInfo, DependentActorGlobalData, ConnectionActorInfoMap, ConnectionManager, FrameNum);
        }
    }

    if (bIsTrafficActorDiscovery && !bIsActorDiscoveryBudgetFull)
    {
        ConnectionManager.GetQueuedBitsForActorDiscovery() += (int32)BitsWritten;
        NetConnection->GetQueuedBits() -= (int32)BitsWritten;
        BitsWritten = 0;
    }

    return BitsWritten;
}

void UReplicationGraph::HandleStarvedActorList(const FPrioritizedRepList& List, int32 StartIdx, FPerConnectionActorInfoMap& ConnectionActorInfoMap, uint32 FrameNum)
{
    for (int32 ActorIdx = StartIdx; ActorIdx < List.Items.Num(); ++ActorIdx)
    {
        const FPrioritizedRepList::FItem& RepItem = List.Items[ActorIdx];
        FConnectionReplicationActorInfo& ActorInfo = *RepItem.ConnectionData;
        FGlobalActorReplicationInfo& GlobalActorInfo = GetGlobalActorReplicationInfoMap().Get(RepItem.Actor);
        auto& DependentActorList = GlobalActorInfo.GetDependentActorList();

        if (DependentActorList.Num() > 0)
        {
            const uint32 CloseFrameNum = ActorInfo.ActorChannelCloseFrameNum;

            for (AActor* DependentActor : DependentActorList)
            {
                FConnectionReplicationActorInfo& DependentActorConnectionInfo = ConnectionActorInfoMap.FindOrAdd(DependentActor);
                DependentActorConnectionInfo.ActorChannelCloseFrameNum = CloseFrameNum > DependentActorConnectionInfo.ActorChannelCloseFrameNum ? CloseFrameNum : DependentActorConnectionInfo.ActorChannelCloseFrameNum;
            }
        }
    }
}

void UReplicationGraph::UpdateActorChannelCloseFrameNum(AActor* Actor, FConnectionReplicationActorInfo& ConnectionData, const FGlobalActorReplicationInfo& GlobalData, const uint32 FrameNum, UNetConnection* NetConnection)
{
    if (GlobalData.GetActorChannelFrameTimeout() > 0)
    {
        const uint32 NewCloseFrameNum = FrameNum + ConnectionData.ReplicationPeriodFrame + GlobalData.GetActorChannelFrameTimeout() + GetGlobalActorChannelFrameNumTimeout();
        ConnectionData.ActorChannelCloseFrameNum = ConnectionData.ActorChannelCloseFrameNum > NewCloseFrameNum ? ConnectionData.ActorChannelCloseFrameNum : NewCloseFrameNum;
    }
}

bool UReplicationGraph::ProcessRemoteFunction(AActor* Actor, UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack, UObject* SubObject)
{
    DETOUR(UReplicationGraph, ProcessRemoteFunction, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x30)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "mov al, 0x1; ret*")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    if (!IsActorValidForReplication(Actor))
        return true;

    while (Function->GetSuperStruct())
        Function = (UFunction*)Function->GetSuperStruct();

    UObject* TargetObj = SubObject ? SubObject : Actor;
    UNetDriver* NetDriver = GetNetDriver();

    const FClassNetCache* ClassCache = NetDriver->GetNetCache()->GetClassNetCache(TargetObj->GetClass());

    if (!ClassCache)
        return true;

    const FFieldNetCache* FieldCache = ClassCache->GetFromField(Function);

    if (!FieldCache)
        return true;

    if (GetFastSharedReplicationBunch() && GetFastSharedReplicationFuncName() == Function->GetFName())
    {
        UActorChannel* FastSharedReplicationChannel = GetFastSharedReplicationChannel();

        if (FastSharedReplicationChannel)
        {
            GetFastSharedReplicationBunch()->Reset();

            auto TempWriter = new FNetBitWriter(nullptr, 0);
            auto RepLayout = NetDriver->GetFunctionRepLayout(Function);

            RepLayout->SendPropertiesForRPC(Function, FastSharedReplicationChannel, *TempWriter, Parameters);

            auto TempBlockWriter = new FNetBitWriter(nullptr, 0);

            FastSharedReplicationChannel->WriteFieldHeaderAndPayload(*TempBlockWriter, ClassCache, FieldCache, nullptr, *TempWriter, true);
            FastSharedReplicationChannel->WriteContentBlockPayload(TargetObj, *(FNetBitWriter*)GetFastSharedReplicationBunch(), false, *TempBlockWriter);

            GetFastSharedReplicationBunch() = nullptr;
            GetFastSharedReplicationChannel() = nullptr;
            SetFastSharedReplicationFuncName(FName(NAME_None));

            delete TempBlockWriter;
            delete TempWriter;
        }

        return true;
    }

    if (Function->GetFunctionFlags() & FUNC_NetMulticast)
    {
        auto RepLayout = NetDriver->GetFunctionRepLayout(Function);

        bool bHasActorLocation = false;
        FVector ActorLocation;

        UNetDriver::ERemoteFunctionSendPolicy SendPolicy = UNetDriver::Default;

        if (FRPCSendPolicyInfo* FuncSendPolicy = GetRPCSendPolicyMap().Find(FObjectKey(Function)))
        {
            if (FuncSendPolicy->bSendImmediately)
                SendPolicy = UNetDriver::ForceSend;
        }

        RepLayout->BuildSharedSerializationForRPC(Parameters);

        FGlobalActorReplicationInfo& GlobalInfo = GetGlobalActorReplicationInfoMap().Get(Actor);

        bool ForceFlushNetDormancy = false;

        FNewReplicatedActorInfo NewActorInfo(Actor);
        const FName ActorStreamingLevelName = NewActorInfo.StreamingLevelName;
        EProcessRemoteFunctionFlags RemoteFunctionFlags = EProcessRemoteFunctionFlags::None;

        for (UNetReplicationGraphConnection* Manager : GetConnections())
        {
            FConnectionReplicationActorInfo& ConnectionActorInfo = Manager->GetActorInfoMap().FindOrAdd(Actor);
            UNetConnection* NetConnection = Manager->GetNetConnection();

            if (NetConnection->GetViewTarget() == nullptr)
                continue;

            if (ActorStreamingLevelName != NAME_None && NetConnection->GetClientVisibleLevelNames().Contains(ActorStreamingLevelName) == false)
                continue;

            if (ConnectionActorInfo.Channel == nullptr && (GetRPC_Multicast_OpenChannelForClass().GetChecked(Actor->GetClass()) == true))
            {
                bool bShouldOpenChannel = true;

                if (ConnectionActorInfo.CullDistanceSquared > 0.f)
                {
                    bShouldOpenChannel = false;

                    if (bHasActorLocation == false)
                    {
                        ActorLocation = Actor->GetActorLocation();
                        bHasActorLocation = true;
                    }

                    FNetViewerArray ViewsToConsider;
                    ViewsToConsider.Emplace(NetConnection, 0.f);

                    auto& Children = NetConnection->GetChildren();

                    for (int32 ChildIdx = 0; ChildIdx < Children.Num(); ++ChildIdx)
                    {
                        if (Children[ChildIdx]->GetViewTarget() != nullptr)
                            ViewsToConsider.Emplace(Children[ChildIdx], 0.f);
                    }

                    for (const FNetViewer& Viewer : ViewsToConsider)
                    {
                        const float DistSq = (ActorLocation - Viewer.GetViewLocation()).SizeSquared();

                        if (DistSq <= ConnectionActorInfo.CullDistanceSquared)
                        {
                            bShouldOpenChannel = true;
                            break;
                        }
                    }
                }

                if (bShouldOpenChannel)
                {
                    ConnectionActorInfo.Channel = (UActorChannel*)NetConnection->CreateChannelByName(FName(NAME_Actor), EChannelCreateFlags::OpenedLocally);

                    if (ConnectionActorInfo.Channel)
                    {
                        ConnectionActorInfo.Channel->SetChannelActor(Actor, ESetChannelActorFlags::None);

                        UpdateActorChannelCloseFrameNum(Actor, ConnectionActorInfo, GlobalInfo, GetReplicationGraphFrame() + 1, NetConnection);

                        ForceFlushNetDormancy |= ConnectionActorInfo.bDormantOnConnection != 0;
                    }
                }
            }

            if (ConnectionActorInfo.Channel)
            {
                NetDriver->ProcessRemoteFunctionForChannelPrivate(ConnectionActorInfo.Channel, ClassCache, FieldCache, TargetObj, NetConnection, Function, Parameters, OutParms, Stack, true, SendPolicy, RemoteFunctionFlags);

                if (SendPolicy == UNetDriver::ForceSend)
                    GetConnectionsNeedingsPostTickDispatchFlush().AddUnique(NetConnection);
            }
        }

        RepLayout->ClearSharedSerializationForRPC();

        if (ForceFlushNetDormancy)
            Actor->FlushNetDormancy();

        return true;
    }

    UNetConnection* Connection = Actor->GetNetConnection();

    if (Connection)
    {
        const bool bIsReliable = (Function->GetFunctionFlags() & FUNC_NetReliable) != 0;

        if (!(bIsReliable || Connection->IsNetReady(false)))
            return true;

        if (Connection->GetUChildConnection())
            Connection = ((UChildConnection*)Connection)->GetParent();

        if (Connection->GetState() == USOCK_Closed)
            return true;

        UActorChannel* Ch = Connection->GetActorChannels().FindRef(TWeakObjectPtr<AActor>(Actor));

        if (Ch == nullptr)
        {
            if (Actor->IsPendingKillPending() || !NetDriver->IsLevelInitializedForActor(Actor, Connection))
                return true;

            Ch = (UActorChannel*)Connection->CreateChannelByName(FName(NAME_Actor), EChannelCreateFlags::OpenedLocally);
            Ch->SetChannelActor(Actor, ESetChannelActorFlags::None);

            if (UNetReplicationGraphConnection* ConnectionManager = Cast<UNetReplicationGraphConnection>(Connection->GetReplicationConnectionDriver()))
            {
                FConnectionReplicationActorInfo& ConnectionActorInfo = ConnectionManager->GetActorInfoMap().FindOrAdd(Actor);
                FGlobalActorReplicationInfo& GlobalInfo = GetGlobalActorReplicationInfoMap().Get(Actor);

                UpdateActorChannelCloseFrameNum(Actor, ConnectionActorInfo, GlobalInfo, GetReplicationGraphFrame() + 1, Connection);
            }
        }

        EProcessRemoteFunctionFlags RemoteFunctionFlags = EProcessRemoteFunctionFlags::None;

        NetDriver->ProcessRemoteFunctionForChannelPrivate(Ch, ClassCache, FieldCache, TargetObj, Connection, Function, Parameters, OutParms, Stack, true, UNetDriver::Default, RemoteFunctionFlags);
    }

    return true;
}

void UReplicationGraph::NotifyConnectionSaturated(UNetReplicationGraphConnection& Connection)
{
    GetWasConnectionSaturated() = true;
}

FGlobalActorReplicationInfoMap& UReplicationGraph::GetGlobalActorReplicationInfoMap()
{
    static auto GlobalActorReplicationInfoMap = Anchor::Offset(TEXT("UReplicationGraph::GlobalActorReplicationInfoMap"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x10)
        .Find("lea rcx, [*+0x*]; lea rdx, [*]; call 0x*")
        .Disp();

    return *(FGlobalActorReplicationInfoMap*)((uint8*)this + GlobalActorReplicationInfoMap);
}

TMap<FObjectKey, FRPCSendPolicyInfo>& UReplicationGraph::GetRPCSendPolicyMap()
{
    return *(TMap<FObjectKey, FRPCSendPolicyInfo>*)(&GetActiveNetworkActors() + 1);
}

TClassMap<bool>& UReplicationGraph::GetRPC_Multicast_OpenChannelForClass()
{
    return *(TClassMap<bool>*)(&GetRPCSendPolicyMap() + 1);
}

TSet<AActor*>& UReplicationGraph::GetActiveNetworkActors()
{
    static auto ActiveNetworkActors = Anchor::Offset(TEXT("UReplicationGraph::ActiveNetworkActors"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x10)
        .Find("add rcx, 0x*")
        .Imm();

    return *(TSet<AActor*>*)((uint8*)this + ActiveNetworkActors);
}

void UReplicationGraph::InitForNetDriver(UNetDriver* InNetDriver)
{
    DETOUR(UReplicationGraph, InitForNetDriver, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x48)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    GetNetDriver() = InNetDriver;

    InitGlobalActorClassSettings();
    InitGlobalGraphNodes();

    for (auto ClientConnection : GetNetDriver()->GetClientConnections())
        AddClientConnection(ClientConnection);
}

void UReplicationGraph::InitializeActorsInWorld(UWorld* InWorld)
{
    DETOUR(UReplicationGraph, InitializeActorsInWorld, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x40)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    static auto InitializeForWorld = Anchor::VirtualFunction<void (*)(UReplicationGraph*, UWorld*)>(TEXT("UReplicationGraph::InitializeForWorld"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x80)
        .Get();

    if (!InWorld)
        return;

    if (InWorld->AreActorsInitialized())
    {
        InitializeForWorld(this, InWorld);
    }
    else
    {
        InWorld->GetOnActorsInitialized().AddLambda([this](const UWorld::FActorsInitializedParams& P)
        {
            InitializeForWorld(this, P.World);
        });
    }
}

void UReplicationGraph::AddClientConnection(UNetConnection* NetConnection)
{
    DETOUR(UReplicationGraph, AddClientConnection, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x28)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    if (NetConnection->GetUChildConnection())
        return;

    auto& PendingConnectionManagers = GetPendingConnectionManagers();

    for (int32 Index = PendingConnectionManagers.Num() - 1; Index >= 0; --Index)
    {
        auto ConnectionManager = PendingConnectionManagers[Index];

        if (ConnectionManager && ConnectionManager->GetNetConnection() == NetConnection)
        {
            PendingConnectionManagers.RemoveAt(Index, 1, false);
            GetConnections().Add(ConnectionManager);

            return;
        }
    }

    GetConnections().Add(CreateClientConnectionManagerInternal(NetConnection));
}

void UReplicationGraph::InitGlobalActorClassSettings()
{
    static auto InitGlobalActorClassSettings = Anchor::VirtualFunction<void (*)(UReplicationGraph*)>(TEXT("UReplicationGraph::InitGlobalActorClassSettings"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x90)
        .Get();

    InitGlobalActorClassSettings(this);
}

void UReplicationGraph::InitGlobalGraphNodes()
{
    static auto InitGlobalGraphNodes = Anchor::VirtualFunction<void (*)(UReplicationGraph*)>(TEXT("UReplicationGraph::InitGlobalGraphNodes"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x98)
        .Get();

    InitGlobalGraphNodes(this);
}

void UReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
    static auto InitConnectionGraphNodes = Anchor::VirtualFunction<void (*)(UReplicationGraph*, UNetReplicationGraphConnection*)>(TEXT("UReplicationGraph::InitConnectionGraphNodes"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0xA0)
        .Get();

    InitConnectionGraphNodes(this, ConnectionManager);
}

void UReplicationGraph::AddNetworkActor(AActor* Actor)
{
    static auto AddNetworkActor = Anchor::VirtualFunction<void (*)(UReplicationGraph*, AActor*)>(TEXT("UReplicationGraph::AddNetworkActor"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x18)
        .Get();

    AddNetworkActor(this, Actor);
}

void UReplicationGraph::InitializeForWorld(UWorld* World)
{
    DETOUR(UReplicationGraph, InitializeForWorld, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x80)
        .IsVirtual(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UReplicationGraph::StaticClass()));

    GetActiveNetworkActors().Reset();
    GetGlobalActorReplicationInfoMap().ResetActorMap();

    for (auto Node : GetGlobalGraphNodes())
        Node->NotifyResetAllNetworkActors();

    for (auto ConnectionManager : GetConnections())
        ConnectionManager->NotifyResetAllNetworkActors();

    if (!World)
        return;

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);

    for (auto Actor : Actors)
    {
        if (Actor && !Actor->IsPendingKill() && ULevel::IsNetActor(Actor))
            AddNetworkActor(Actor);
    }
}

UNetReplicationGraphConnection* UReplicationGraph::CreateClientConnectionManagerInternal(UNetConnection* Connection)
{
    auto ConnectionManagerClass = GetReplicationConnectionManagerClass();
    auto NewConnectionManager = NewObject<UNetReplicationGraphConnection>(this, ConnectionManagerClass && ConnectionManagerClass->IsChildOf(UNetReplicationGraphConnection::StaticClass()) ? ConnectionManagerClass : nullptr);

    NewConnectionManager->GetConnectionOrderNum() = GetConnections().Num() + GetPendingConnectionManagers().Num();
    NewConnectionManager->GetConnectionId() = NewConnectionManager->GetConnectionOrderNum();
    NewConnectionManager->InitForGraph(this);
    NewConnectionManager->InitForConnection(Connection);

    InitConnectionGraphNodes(NewConnectionManager);

    return NewConnectionManager;
}
