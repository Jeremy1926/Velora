#include "pch.h"
#include "ReplicationGraph.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/Level.h"
#include "Engine/PackageMapClient.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "Kismet/GameplayStatics.h"
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
