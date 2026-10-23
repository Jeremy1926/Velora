#include "pch.h"
#include "FortReplicationGraph.h"
#include "FortTeamActorInterface.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/BitArray.h"
#include "GameFramework/PlayerState.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Pawns/FortAIPawn.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortLiveBroadcastController.h"
#include "Player/FortLiveSpectatorController.h"
#include "Player/FortPlayerState.h"
#include "Player/FortTeamHealthInfo.h"
#include "HAL/IConsoleManager.h"
#include "HAL/UnrealMemory.h"
#include "Templates/Casts.h"
#include "Templates/Function.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/Stack.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortReplicationGraph::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraph"));

    return Class;
}

UReplicationGraphNode*& UFortReplicationGraph::GetPlayerStateLimiterNode()
{
    static auto PlayerStateLimiterNode = StaticClass()->FindPropertyByName(FName(TEXT("PlayerStateLimiterNode")));

    return *PlayerStateLimiterNode->ContainerPtrToValuePtr<UReplicationGraphNode*>(this);
}

UReplicationGraphNode*& UFortReplicationGraph::GetReplayPlayerStateLimiterNode()
{
    static auto ReplayPlayerStateLimiterNode = StaticClass()->FindPropertyByName(FName(TEXT("ReplayPlayerStateLimiterNode")));

    return *ReplayPlayerStateLimiterNode->ContainerPtrToValuePtr<UReplicationGraphNode*>(this);
}

UReplicationGraphNode_GridSpatialization2D*& UFortReplicationGraph::GetRootGridNode()
{
    static auto RootGridNode = StaticClass()->FindPropertyByName(FName(TEXT("RootGridNode")));

    return *RootGridNode->ContainerPtrToValuePtr<UReplicationGraphNode_GridSpatialization2D*>(this);
}

TMap<UClass*, UReplicationGraphNode*>& UFortReplicationGraph::GetClassToNodetoRouteTo()
{
    static auto ClassToNodetoRouteTo = StaticClass()->FindPropertyByName(FName(TEXT("ClassToNodetoRouteTo")));

    return *ClassToNodetoRouteTo->ContainerPtrToValuePtr<TMap<UClass*, UReplicationGraphNode*>>(this);
}

bool& UFortReplicationGraph::GetDisableSquadNodes()
{
    return *(bool*)(&GetClassToNodetoRouteTo() + 1);
}

bool& UFortReplicationGraph::GetDisableHighFrequencyPawnRouting()
{
    return *(&GetDisableSquadNodes() + 1);
}

TClassMap<EClassRepNodeMapping>& UFortReplicationGraph::GetClassRepNodePolicies()
{
    static auto ClassRepNodePolicies = Anchor::Offset(TEXT("UFortReplicationGraph::ClassRepNodePolicies"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), 0x50)
        .Find("mov rbx, rcx; add rcx, 0x*")
        .Find("add rcx, 0x*")
        .Or()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), 0x50)
        .Find("call 0x*")
        .Target()
        .Find("add rcx, 0x*")
        .Imm();

    return *(TClassMap<EClassRepNodeMapping>*)((uint8*)this + ClassRepNodePolicies);
}

TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>& UFortReplicationGraph::GetSquadMap()
{
    return *(TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>*)((uint8*)(&GetClassRepNodePolicies() + 1) + 2 * sizeof(TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>));
}

TMap<FName, FActorRepListRefView>& UFortReplicationGraph::GetAlwaysRelevantStreamingLevelActors()
{
    static auto ReplayPlayerStateLimiterNode = StaticClass()->FindPropertyByName(FName(TEXT("ReplayPlayerStateLimiterNode")));

    return *(TMap<FName, FActorRepListRefView>*)(ReplayPlayerStateLimiterNode->ContainerPtrToValuePtr<uint8>(this) + sizeof(UReplicationGraphNode*));
}

void UFortReplicationGraph::InitializeForWorld(UWorld* World)
{
    DETOUR(UFortReplicationGraph, InitializeForWorld, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x80)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "push rbx; sub rsp, 0x20; lea rbx, [rcx+0x*]; mov eax, [rbx+0x8]; cmp eax, [rbx+0x34]; jz 0x*; mov rcx, rbx; call 0x*; mov rcx, rbx; call 0x*; add rsp, 0x20; pop rbx; ret")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    Original(World);

    UReplicationGraph::InitializeForWorld(World);
}

void UFortReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager)
{
    DETOUR(UFortReplicationGraph, InitConnectionGraphNodes, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0xA0)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "mov [rsp+0x8], rbx; mov [rsp+0x10], rsi; push rdi; sub rsp, 0x20; mov rdi, rdx; mov rbx, rcx; call 0x*; mov r8, [rbx]; mov rdx, rax; mov rcx, rbx; mov rsi, rax; call [r8+0x*]")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    Original(ConnectionManager);

    auto AlwaysRelevantConnectionNode = CreateNewNode<UFortReplicationGraphNode_AlwaysRelevant_ForConnection>();

    ConnectionManager->GetOnClientVisibleLevelNameAdd().AddUObject(AlwaysRelevantConnectionNode, &UFortReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd);
    ConnectionManager->GetOnClientVisibleLevelNameRemove().AddUObject(AlwaysRelevantConnectionNode, &UFortReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove);

    AddConnectionGraphNode(AlwaysRelevantConnectionNode, ConnectionManager);
    AddConnectionGraphNode(ConnectionManager->GetNetConnection()->IsReplay() ? GetReplayPlayerStateLimiterNode() : GetPlayerStateLimiterNode(), ConnectionManager);
}

void UFortReplicationGraph::EnableDynamicSpatialFrequencyNode(bool bEnable)
{
    if (bEnable)
    {
        UFortReplicationGraphNode_DynamicSpatialFrequency::GetAIClasses().Set(AActor::StaticClass(), false);
        UFortReplicationGraphNode_DynamicSpatialFrequency::GetAIClasses().Set(AFortAIPawn::StaticClass(), true);

        GetRootGridNode()->GetCreateCellNodeOverride() = [](UReplicationGraphNode_GridSpatialization2D* Parent) -> UReplicationGraphNode_GridCell*
        {
            auto NewNode = Parent->CreateChildNode<UReplicationGraphNode_GridCell>();

            NewNode->GetCreateDynamicNodeOverride() = [](UReplicationGraphNode_GridCell* CellNode) -> UReplicationGraphNode*
            {
                auto DynamicNode = CellNode->CreateChildNode<UFortReplicationGraphNode_DynamicSpatialFrequency>();

                if (DynamicNode)
                    DynamicNode->SetupClassToNodeMap();

                return DynamicNode;
            };

            return NewNode;
        };
    }
    else
    {
        GetRootGridNode()->GetCreateCellNodeOverride() = nullptr;
    }

    InitDynamicSpatializationSettings();
}

void UFortReplicationGraph::InitDynamicSpatializationSettings()
{
    static auto PlayerPawnKBytesSec = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.PlayerPawnKBytesSec"));
    static auto AIPawnKBytesSec = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.AIPawnKBytesSec"));
    static auto MaxAIPerClient = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.Deimos.MaxAIPerClient"));

    using FSpatializationZone = UReplicationGraphNode_DynamicSpatialFrequency::FSpatializationZone;
    using FSettings = UReplicationGraphNode_DynamicSpatialFrequency::FSettings;

    static TArray<FSpatializationZone> DefaultZones;
    static TArray<FSpatializationZone> PhysicsObjectZones;
    static TArray<FSpatializationZone> VehicleZones;
    static TArray<FSpatializationZone> AIZones;
    static TArray<FSpatializationZone> NonFastSharedZones;

    const float TickRate = (float)GetNetDriver()->GetNetServerMaxTickRate();

    DefaultZones.Reset();
    PhysicsObjectZones.Reset();
    VehicleZones.Reset();
    AIZones.Reset();
    NonFastSharedZones.Reset();

    DefaultZones.Add(FSpatializationZone(0.f, 0.1f, 0.15f, 1.f, 1.f, 5.f, 3.f, TickRate));
    DefaultZones.Add(FSpatializationZone(0.71f, 0.1f, 0.25f, 1.f, 1.f, 10.f, 5.f, TickRate));
    DefaultZones.Add(FSpatializationZone(1.f, 0.1f, 0.5f, 1.f, 1.f, 20.f, 10.f, TickRate));

    AIZones.Add(FSpatializationZone(0.f, 0.05f, 0.15f, 1.f, 1.f, 5.f, 2.f, TickRate));
    AIZones.Add(FSpatializationZone(0.71f, 0.05f, 0.25f, 1.f, 1.f, 5.f, 3.f, TickRate));
    AIZones.Add(FSpatializationZone(1.f, 0.1f, 0.5f, 1.f, 1.f, 10.f, 5.f, TickRate));

    NonFastSharedZones.Add(FSpatializationZone(0.f, 0.1f, 0.5f, 10.f, 5.f, 0.f, 0.f, TickRate));
    NonFastSharedZones.Add(FSpatializationZone(0.71f, 0.1f, 0.5f, 15.f, 10.f, 0.f, 0.f, TickRate));
    NonFastSharedZones.Add(FSpatializationZone(1.f, 0.1f, 0.5f, 30.f, 10.f, 0.f, 0.f, TickRate));

    PhysicsObjectZones.Add(FSpatializationZone(0.f, 0.1f, 0.15f, 5.f, 1.f, 5.f, 1.f, TickRate));
    PhysicsObjectZones.Add(FSpatializationZone(0.71f, 0.1f, 0.25f, 5.f, 5.f, 5.f, 5.f, TickRate));
    PhysicsObjectZones.Add(FSpatializationZone(1.f, 0.1f, 0.5f, 10.f, 5.f, 10.f, 5.f, TickRate));

    VehicleZones.Add(FSpatializationZone(0.f, 0.1f, 0.15f, 5.f, 1.f, 5.f, 1.f, TickRate));
    VehicleZones.Add(FSpatializationZone(0.71f, 0.1f, 0.25f, 10.f, 5.f, 10.f, 5.f, TickRate));
    VehicleZones.Add(FSpatializationZone(1.f, 0.1f, 0.5f, 15.f, 5.f, 15.f, 5.f, TickRate));

    const int64 PlayerPawnMaxBitsPerFrame = (int64)(PlayerPawnKBytesSec->GetFloat() * 1024.f * 8.f * (1.f / TickRate));
    const int64 AIPawnMaxBitsPerFrame = (int64)(AIPawnKBytesSec->GetFloat() * 1024.f * 8.f * (1.f / TickRate));
    auto NodeSettings = UFortReplicationGraphNode_DynamicSpatialFrequency::GetNodeSettings();

    UReplicationGraphNode_DynamicSpatialFrequency::GetDefaultSettings() = FSettings{ { DefaultZones.GetData(), DefaultZones.Num() }, { NonFastSharedZones.GetData(), NonFastSharedZones.Num() }, PlayerPawnMaxBitsPerFrame, -1 };
    NodeSettings[(uint8)EFortDynamicNodeType::WorldStatic] = FSettings{ { AIZones.GetData(), AIZones.Num() }, { NonFastSharedZones.GetData(), NonFastSharedZones.Num() }, AIPawnMaxBitsPerFrame, MaxAIPerClient->GetInt() };
    NodeSettings[(uint8)EFortDynamicNodeType::WorldDynamic] = FSettings{ { PhysicsObjectZones.GetData(), PhysicsObjectZones.Num() }, { PhysicsObjectZones.GetData(), PhysicsObjectZones.Num() }, PlayerPawnMaxBitsPerFrame, -1 };
    NodeSettings[(uint8)EFortDynamicNodeType::Pawn] = FSettings{ { VehicleZones.GetData(), VehicleZones.Num() }, { VehicleZones.GetData(), VehicleZones.Num() }, PlayerPawnMaxBitsPerFrame, -1 };
}

void UFortReplicationGraph::InitForPlaylist(AFortGameModeAthena* GameMode)
{
    static auto PlaylistAthenaClass = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena"));
    static auto ServerMaxTickRate = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("ServerMaxTickRate")));
    static auto bOverrideServerPerClientMaxAI = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("bOverrideServerPerClientMaxAI")));
    static auto ServerPerClientMaxAI = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("ServerPerClientMaxAI")));
    static auto NetActorDiscoveryBudgetInKBytesPerSec = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("NetActorDiscoveryBudgetInKBytesPerSec")));
    static auto NetDormancyNumFramesUntilObsolete = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("NetDormancyNumFramesUntilObsolete")));
    static auto RespawnType = PlaylistAthenaClass->FindPropertyByName(FName(TEXT("RespawnType")));
    static auto PlaylistName = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("PlaylistName")));
    static auto ConnectionDormancyNumFramesUntilObsolete = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.ConnectionDormancy.NumFramesUntilObsolete"));
    static auto TargetKBitsSecFastSharedPath = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.TargetKBitsSecFastSharedPath"));
    static auto OverrideActorDiscoveryBudget = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.OverrideActorDiscoveryBudget"));
    static auto EnableDynamicDormancyActorDestructionForCreativeModes = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.EnableDynamicDormancyActorDestructionForCreativeModes"));
    static auto DSFEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DSF.Enabled"));
    static auto DynamicActorFrequencyBuckets = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DynamicActorFrequencyBuckets"));
    static auto DynamicActorFrequencyBucketsBigTeamMode = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DynamicActorFrequencyBuckets.BigTeamMode"));
    static auto FastSharedPathFrameModulo = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.FastSharedPathFrameModulo"));
    static auto InstantFlushLargeTeamMode = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.InstantFlush.LargeTeamMode"));

    uint32 NumFramesUntilObsolete = ConnectionDormancyNumFramesUntilObsolete->GetInt();
    bool bIsLargeTeamGame = false;

    if (auto GameState = GameMode ? Cast<AFortGameStateAthena>(GameMode->GetGameState()) : nullptr)
    {
        bIsLargeTeamGame = GameState->GetIsLargeTeamGame();

        const bool bIsCreativeOrPlaygroundModePlaylist = GameState->IsCreativeOrPlaygroundModePlaylist();
        const bool bIsBattleLabModePlaylist = GameState->IsBattleLabModePlaylist();
        auto Playlist = GameState->GetCurrentPlaylist();
        auto NetDriver = GetNetDriver();

        if (NetDriver && Playlist && *ServerMaxTickRate->ContainerPtrToValuePtr<uint8>(Playlist))
        {
            int32 NewNetServerMaxTickRate = NetDriver->GetNetServerMaxTickRate();

            if (*ServerMaxTickRate->ContainerPtrToValuePtr<uint8>(Playlist) == 1)
                NewNetServerMaxTickRate = 20;
            else if (*ServerMaxTickRate->ContainerPtrToValuePtr<uint8>(Playlist) == 2)
                NewNetServerMaxTickRate = 30;

            NetDriver->SetNetServerMaxTickRate(NewNetServerMaxTickRate);

            InitDynamicSpatializationSettings();

            GetFastSharedPathConstants().MaxBitsPerFrame = (int32)((float)TargetKBitsSecFastSharedPath->GetInt() / (float)NetDriver->GetNetServerMaxTickRate());
        }

        if (OverrideActorDiscoveryBudget->GetInt() >= 0)
            SetActorDiscoveryBudget(OverrideActorDiscoveryBudget->GetInt());
        else if (Playlist && *NetActorDiscoveryBudgetInKBytesPerSec->ContainerPtrToValuePtr<int32>(Playlist) > 0)
            SetActorDiscoveryBudget(*NetActorDiscoveryBudgetInKBytesPerSec->ContainerPtrToValuePtr<int32>(Playlist));

        if (Playlist && *NetDormancyNumFramesUntilObsolete->ContainerPtrToValuePtr<int32>(Playlist) >= 0)
            NumFramesUntilObsolete = *NetDormancyNumFramesUntilObsolete->ContainerPtrToValuePtr<int32>(Playlist);
        else if (bIsCreativeOrPlaygroundModePlaylist || (Playlist && *RespawnType->ContainerPtrToValuePtr<uint8>(Playlist)))
            NumFramesUntilObsolete = 0;

        if (bIsCreativeOrPlaygroundModePlaylist && !bIsBattleLabModePlaylist)
        {
            if (auto DormantDynamicActorsDestruction = IConsoleManager::Get().FindConsoleVariable(TEXT("Net.RepGraph.DormantDynamicActorsDestruction")))
                DormantDynamicActorsDestruction->Set(EnableDynamicDormancyActorDestructionForCreativeModes->GetInt(), ECVF_SetByCode);
        }

        if (Playlist && *bOverrideServerPerClientMaxAI->ContainerPtrToValuePtr<bool>(Playlist))
            UFortReplicationGraphNode_DynamicSpatialFrequency::GetNodeSettings()[(uint8)EFortDynamicNodeType::WorldStatic].MaxNearestActors = *ServerPerClientMaxAI->ContainerPtrToValuePtr<int32>(Playlist);

        if (GameState->GetPlaylistDataIsLoaded())
        {
            const FGameplayTagContainer GameplayTags = GameState->GetAthenaPlaylistContextTags();

            OnGameStatePlaylistLoaded(PlaylistName->GetPropertyValue_InContainer<FName>(GameState->GetCurrentPlaylist()), GameplayTags);
        }
        else
        {
            FScriptDelegate OnCurrentPlaylistLoadedDelegate;
            OnCurrentPlaylistLoadedDelegate.BindUFunction(this, FName(TEXT("OnGameStatePlaylistLoaded")));

            GameState->GetOnCurrentPlaylistLoaded().AddUnique(OnCurrentPlaylistLoadedDelegate);
        }
    }

    if (!DSFEnabled->GetInt())
    {
        auto& DefaultSettings = UReplicationGraphNode_ActorListFrequencyBuckets::GetDefaultSettings();
        uint32 MinActorChannelFrameNumTimeout;

        if (!bIsLargeTeamGame)
        {
            DefaultSettings.NumBuckets = DynamicActorFrequencyBuckets->GetInt();
            DefaultSettings.BucketThresholds.Reset();
            DefaultSettings.EnableFastPath = true;
            DefaultSettings.FastPathFrameModulo = 1;
            MinActorChannelFrameNumTimeout = DynamicActorFrequencyBuckets->GetInt() * 2;
        }
        else
        {
            DefaultSettings.BucketThresholds.Add({ 50, DynamicActorFrequencyBucketsBigTeamMode->GetInt() });
            DefaultSettings.BucketThresholds.Add({ 80, DynamicActorFrequencyBucketsBigTeamMode->GetInt() + 1 });
            DefaultSettings.BucketThresholds.Add({ 999, DynamicActorFrequencyBucketsBigTeamMode->GetInt() + 2 });
            DefaultSettings.FastPathFrameModulo = FastSharedPathFrameModulo->GetInt();
            MinActorChannelFrameNumTimeout = DynamicActorFrequencyBucketsBigTeamMode->GetInt() + 2;
        }

        GetGlobalActorChannelFrameNumTimeout() = FMath::Max<uint32>(GetGlobalActorChannelFrameNumTimeout(), MinActorChannelFrameNumTimeout);
    }

    auto& RPCSendPolicyMap = GetRPCSendPolicyMap();
    RPCSendPolicyMap.Reset();

    if (!bIsLargeTeamGame || InstantFlushLargeTeamMode->GetInt() > 0)
        RPCSendPolicyMap.Add(FObjectKey(AFortPlayerPawnAthena::StaticClass()->FindFunctionByName(FName(TEXT("NetMulticast_Athena_BatchedDamageCues")))), FRPCSendPolicyInfo{ true });

    RPCSendPolicyMap.Add(FObjectKey(AFortAIPawn::StaticClass()->FindFunctionByName(FName(TEXT("NetMulticast_BatchedDamageCues")))), FRPCSendPolicyInfo{ true });

    UReplicationGraphNode_ConnectionDormancyNode::SetNumFramesUntilObsolete(NumFramesUntilObsolete);
}

void UFortReplicationGraph::AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping)
{
    if (Mapping >= EClassRepNodeMapping::Spatialize_Static)
        Class->GetDefaultObject(true);

    GetClassRepNodePolicies().Set(Class, Mapping);
}

void UFortReplicationGraph::SetAllSquadNodesEnabled(bool bEnabled)
{
    for (const auto& Pair : GetSquadMap())
    {
        if (auto SquadNode = Pair.Value)
            SquadNode->GetSpecialRelevancyEnabled() = bEnabled;
    }
}

void UFortReplicationGraph::OnGameStatePlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& GameplayTags)
{
    static auto InitializeForWorld = Anchor::VirtualFunction<void (*)(UReplicationGraph*, UWorld*)>(TEXT("UReplicationGraph::InitializeForWorld"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x80)
        .Get();

    auto World = GetNetDriver()->GetWorld();
    auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());
    auto GameState = GameMode ? Cast<AFortGameStateAthena>(GameMode->GetGameState()) : nullptr;

    if (!GameMode || !GameState)
        return;

    GameState->GetOnCurrentPlaylistLoaded().RemoveAll(this);
    GetClassToNodetoRouteTo().Reset();
    GetDisableSquadNodes() = false;
    GetDisableHighFrequencyPawnRouting() = false;

    auto& GlobalRepGraphNodeClassesToAdd = GameMode->GetGlobalRepGraphNodeClassesToAdd();

    if (GlobalRepGraphNodeClassesToAdd.Num() <= 0)
        return;

    for (const auto& ClassMapping : GlobalRepGraphNodeClassesToAdd)
    {
        if (!ClassMapping.NodeClass || !ClassMapping.NodeClass->IsChildOf(UFortReplicationGraphNode_Custom::StaticClass()))
            continue;

        auto CustomNode = (UFortReplicationGraphNode_Custom*)NewObject<UReplicationGraphNode>(this, ClassMapping.NodeClass);

        if (!CustomNode)
            continue;

        InitNode(CustomNode);
        CustomNode->InitializeNode(World);
        AddGlobalGraphNode(CustomNode);

        GetDisableSquadNodes() |= ClassMapping.bShouldDisableSquadNodes;
        GetDisableHighFrequencyPawnRouting() |= ClassMapping.bShouldDisableHighFrequencyPawnRouting;

        for (auto Class : ClassMapping.ClassesToRoute)
        {
            AddClassRepInfo(Class, EClassRepNodeMapping::Custom);
            GetClassToNodetoRouteTo().Add(Class, CustomNode);
        }
    }

    if (GetDisableSquadNodes())
        SetAllSquadNodesEnabled(false);

    InitializeForWorld(this, World);
}

void UFortReplicationGraph::execOnGameStatePlaylistLoaded(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortReplicationGraph, execOnGameStatePlaylistLoaded, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortReplicationGraph:OnGameStatePlaylistLoaded"))
        .Has("setnz al; add [*+0x20], rax; test rcx, rcx; jz 0x*; call 0x*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortReplicationGraph:OnGameStatePlaylistLoaded")));

    FName PlaylistName = NAME_None;
    FGameplayTagContainer GameplayTagsTemp;

    Stack.StepCompiledIn(&PlaylistName);
    const FGameplayTagContainer& GameplayTags = Stack.StepCompiledInRef<FGameplayTagContainer>(&GameplayTagsTemp);
    Stack.Code += !!Stack.Code;

    OnGameStatePlaylistLoaded(PlaylistName, GameplayTags);
}

bool& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetSpecialRelevancyEnabled()
{
    static auto SpecialRelevancyEnabled = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantForSquad::bSpecialRelevancyEnabled"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"))
        .Find("cmp byte ptr [rbx+0x*], 0x0")
        .Disp();

    return *(bool*)((uint8*)this + SpecialRelevancyEnabled);
}

UClass* UFortReplicationGraphNode_Custom::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_Custom"));

    return Class;
}

void UFortReplicationGraphNode_Custom::InitializeNode(UWorld* World)
{
    static auto InitializeNode = Anchor::VirtualFunction<void (*)(UFortReplicationGraphNode_Custom*, UWorld*)>(TEXT("UFortReplicationGraphNode_Custom::InitializeNode"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_Custom"), 0x18)
        .Get();

    InitializeNode(this, World);
}

UClass* UFortReplicationGraphNode_AlwaysRelevant_ForConnection::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_AlwaysRelevant_ForConnection"));

    return Class;
}

FActorRepListRefView& UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevant_ForConnection::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x20)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevant_ForConnection"))
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
}

int32& UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GetNumAlwaysRelevantStreamingLevelsNeedingReplication()
{
    return *(int32*)((uint8*)&GetReplicationActorList() - 2 * sizeof(int32));
}

uint8* UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GetAlwaysRelevantStreamingLevelsNeedingReplication()
{
    auto SecondaryData = (uint8**)((uint8*)&GetNumAlwaysRelevantStreamingLevelsNeedingReplication() - sizeof(uint8*));

    return *SecondaryData ? *SecondaryData : (uint8*)SecondaryData - 64 * TGameLayout<FName>::Size();
}

void UFortReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld)
{
    const int32 NameSize = TGameLayout<FName>::Size();
    auto& ArrayNum = GetNumAlwaysRelevantStreamingLevelsNeedingReplication();
    auto& ArrayMax = *(&ArrayNum + 1);
    auto& SecondaryData = *(uint8**)((uint8*)&ArrayNum - sizeof(uint8*));
    auto InlineData = (uint8*)&SecondaryData - 64 * NameSize;

    for (int32 Index = 0; Index < ArrayNum; Index++)
    {
        if (TGameLayout<FName>::Load(GetAlwaysRelevantStreamingLevelsNeedingReplication() + Index * NameSize) == LevelName)
            return;
    }

    const int32 OldNum = ArrayNum++;

    if (ArrayNum > ArrayMax)
    {
        SIZE_T Grow = 4;

        if (ArrayMax || SIZE_T(ArrayNum) > Grow)
            Grow = SIZE_T(ArrayNum) + 3 * SIZE_T(ArrayNum) / 8 + 16;

        ArrayMax = ArrayNum <= 64 ? 64 : (int32)(FMemory::QuantizeSize(Grow * NameSize) / NameSize);

        if (ArrayMax > 64)
        {
            if (!SecondaryData)
            {
                SecondaryData = (uint8*)FMemory::Realloc(nullptr, ArrayMax * NameSize);
                memcpy(SecondaryData, InlineData, OldNum * NameSize);
            }
            else
            {
                SecondaryData = (uint8*)FMemory::Realloc(SecondaryData, ArrayMax * NameSize);
            }
        }
    }

    TGameLayout<FName>::Store(GetAlwaysRelevantStreamingLevelsNeedingReplication() + OldNum * NameSize, LevelName);
}

void UFortReplicationGraphNode_AlwaysRelevant_ForConnection::OnClientLevelVisibilityRemove(FName LevelName)
{
    const int32 NameSize = TGameLayout<FName>::Size();
    auto& ArrayNum = GetNumAlwaysRelevantStreamingLevelsNeedingReplication();
    auto Data = GetAlwaysRelevantStreamingLevelsNeedingReplication();
    int32 WriteIndex = 0;

    for (int32 ReadIndex = 0; ReadIndex < ArrayNum; ReadIndex++)
    {
        if (TGameLayout<FName>::Load(Data + ReadIndex * NameSize) == LevelName)
            continue;

        if (WriteIndex != ReadIndex)
            memmove(Data + WriteIndex * NameSize, Data + ReadIndex * NameSize, NameSize);

        WriteIndex++;
    }

    ArrayNum = WriteIndex;
}

TArray<FFortAlwaysRelevantActorInfo>& UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GetPastRelevantActors()
{
    return *(TArray<FFortAlwaysRelevantActorInfo>*)(&GetReplicationActorList() + 1);
}

bool& UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GetInitializedPlayerState()
{
    return *(bool*)(&GetPastRelevantActors() + 1);
}

void UFortReplicationGraphNode_AlwaysRelevant_ForConnection::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UFortReplicationGraphNode_AlwaysRelevant_ForConnection, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevant_ForConnection"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraphNode_AlwaysRelevant_ForConnection::StaticClass()));

    auto Graph = (UFortReplicationGraph*)GetOuter();
    auto& ReplicationActorList = GetReplicationActorList();
    ReplicationActorList.RepList.Reset();

    auto ConnectionManager = Params.GetConnectionManager();
    auto& ActorInfoMap = ConnectionManager->GetActorInfoMap();
    const int32 ConnectionParity = ConnectionManager->GetConnectionOrderNum() % 2;
    const int32 FrameParity = Params.GetReplicationFrameNum() & 1;
    const bool bIsReplay = ConnectionManager->GetNetConnection()->IsReplay();

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        const FNetViewer& CurViewer = Params.GetViewer(Index);

        if (bIsReplay && CurViewer.Connection != ConnectionManager->GetNetConnection())
        {
            if (CurViewer.ViewTarget && !CurViewer.ViewTarget->GetOnlyRelevantToOwner())
                ReplicationActorList.ConditionalAdd(CurViewer.ViewTarget);

            auto PlayerController = Cast<AFortPlayerController>(CurViewer.InViewer);

            if (!PlayerController)
                continue;

            if (ConnectionParity == FrameParity)
            {
                ReplicationActorList.ConditionalAdd((AActor*)((IFortInventoryOwnerInterface*)PlayerController->GetInterfaceAddress(IFortInventoryOwnerInterface::StaticClass()))->GetWorldInventory());

                if (auto PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController))
                {
                    ReplicationActorList.ConditionalAdd((AActor*)PlayerControllerAthena->GetViewTargetInventory());
                    ReplicationActorList.ConditionalAdd((AActor*)PlayerControllerAthena->GetBroadcastRemoteClientInfo());
                }
            }

            if (auto Pawn = Cast<AFortPawn>(PlayerController->GetPawn()))
            {
                auto& ActorInfo = ActorInfoMap.FindOrAdd(Pawn);
                ActorInfo.SetCullDistanceSquared(0.f);
                ActorInfo.ReplicationPeriodFrame = 1;

                if (CurViewer.ViewTarget != Pawn)
                    ReplicationActorList.ConditionalAdd(Pawn);

                for (auto Weapon : Pawn->GetCurrentWeaponList())
                    ReplicationActorList.ConditionalAdd((AActor*)Weapon);
            }

            if (auto ViewTargetPlayerPawn = Cast<AFortPlayerPawn>(CurViewer.ViewTarget))
                ReplicationActorList.ConditionalAdd(ViewTargetPlayerPawn->GetTetherPawn());
            else if (auto ViewTargetAIPawn = Cast<AFortAIPawn>(CurViewer.ViewTarget))
                ReplicationActorList.ConditionalAdd(ViewTargetAIPawn->GetTetheredFollower());

            continue;
        }

        ReplicationActorList.RepList.Add(CurViewer.InViewer);
        ReplicationActorList.ConditionalAdd(CurViewer.ViewTarget);

        auto PlayerController = Cast<AFortPlayerController>(CurViewer.InViewer);

        if (!PlayerController)
            continue;

        if (ConnectionParity == FrameParity)
        {
            if (auto PlayerState = PlayerController->GetPlayerState())
            {
                if (!GetInitializedPlayerState())
                {
                    GetInitializedPlayerState() = true;
                    ActorInfoMap.FindOrAdd(PlayerState).ReplicationPeriodFrame = 1;
                }

                ReplicationActorList.ConditionalAdd(PlayerState);
            }

            ReplicationActorList.ConditionalAdd((AActor*)((IFortInventoryOwnerInterface*)PlayerController->GetInterfaceAddress(IFortInventoryOwnerInterface::StaticClass()))->GetWorldInventory());

            if (auto PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController))
            {
                ReplicationActorList.ConditionalAdd((AActor*)PlayerControllerAthena->GetViewTargetInventory());
                ReplicationActorList.ConditionalAdd((AActor*)PlayerControllerAthena->GetBroadcastRemoteClientInfo());

                auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerControllerAthena->GetPlayerState());

                if (CurViewer.InViewer != CurViewer.ViewTarget && PlayerStateAthena && PlayerStateAthena->IsTeamOutOfGame())
                {
                    if (auto ViewTargetPawn = Cast<AFortPawn>(CurViewer.ViewTarget))
                    {
                        if (auto ViewTargetPlayerState = Cast<AFortPlayerStateAthena>(ViewTargetPawn->GetPlayerState()))
                            ReplicationActorList.ConditionalAdd((AActor*)ViewTargetPlayerState->GetPlayerTeamPrivate());
                    }
                }
            }
        }

        auto& PastRelevantActors = GetPastRelevantActors();
        auto LastData = PastRelevantActors.FindByKey(CurViewer.Connection);

        if (!LastData)
        {
            FFortAlwaysRelevantActorInfo NewActorInfo = { CurViewer.Connection, nullptr, nullptr };
            LastData = &PastRelevantActors[PastRelevantActors.Add(NewActorInfo)];
        }

        if (auto Pawn = Cast<AFortPawn>(PlayerController->GetPawn()))
        {
            if (Pawn != LastData->LastPawn)
            {
                LastData->LastPawn = Pawn;

                auto& ActorInfo = ActorInfoMap.FindOrAdd(Pawn);
                ActorInfo.SetCullDistanceSquared(0.f);
                ActorInfo.ReplicationPeriodFrame = 1;
            }

            if (CurViewer.ViewTarget != Pawn)
                ReplicationActorList.ConditionalAdd(Pawn);

            for (auto Weapon : Pawn->GetCurrentWeaponList())
                ReplicationActorList.ConditionalAdd((AActor*)Weapon);
        }
        else if (CurViewer.InViewer == CurViewer.ViewTarget && LastData->LastPawn)
        {
            ReplicationActorList.ConditionalAdd(LastData->LastPawn);
        }

        AActor* TetherPawn = nullptr;

        if (auto ViewTargetPlayerPawn = Cast<AFortPlayerPawn>(CurViewer.ViewTarget))
        {
            TetherPawn = ViewTargetPlayerPawn->GetTetherPawn();
            ReplicationActorList.ConditionalAdd(TetherPawn);
        }
        else if (auto ViewTargetAIPawn = Cast<AFortAIPawn>(CurViewer.ViewTarget))
        {
            TetherPawn = ViewTargetAIPawn->GetTetheredFollower();
            ReplicationActorList.ConditionalAdd(TetherPawn);
        }

        if (CurViewer.ViewTarget != LastData->LastPawn && Cast<AFortPawn>(CurViewer.ViewTarget))
        {
            auto LastPawn = Cast<AFortPawn>(LastData->LastPawn);

            if (LastPawn && LastPawn != TetherPawn)
                LastPawn->UpdateCullDistance();

            LastData->LastPawn = CurViewer.ViewTarget;

            auto& ActorInfo = ActorInfoMap.FindOrAdd(CurViewer.ViewTarget);
            ActorInfo.SetCullDistanceSquared(0.f);
            ActorInfo.ReplicationPeriodFrame = 1;
        }

        if (TetherPawn)
        {
            if (TetherPawn != LastData->LastTetherPawn)
            {
                LastData->LastTetherPawn = TetherPawn;

                auto& ActorInfo = ActorInfoMap.FindOrAdd(TetherPawn);
                ActorInfo.SetCullDistanceSquared(0.f);
                ActorInfo.ReplicationPeriodFrame = 1;
            }
        }
        else if (LastData->LastTetherPawn)
        {
            auto LastTetherPawn = Cast<AFortPawn>(LastData->LastTetherPawn);

            if (LastTetherPawn && LastTetherPawn != LastData->LastPawn)
                LastTetherPawn->UpdateCullDistance();

            LastData->LastTetherPawn = nullptr;
        }
    }

    GetPastRelevantActors().RemoveAll([](const FFortAlwaysRelevantActorInfo& RelActorInfo)
    {
        return RelActorInfo.Connection == nullptr;
    });

    Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ReplicationActorList);

    auto& AlwaysRelevantStreamingLevelActors = Graph->GetAlwaysRelevantStreamingLevelActors();
    auto& NumAlwaysRelevantStreamingLevels = GetNumAlwaysRelevantStreamingLevelsNeedingReplication();

    for (int32 Idx = NumAlwaysRelevantStreamingLevels - 1; Idx >= 0; Idx--)
    {
        auto StreamingLevels = GetAlwaysRelevantStreamingLevelsNeedingReplication();
        const FName StreamingLevel = TGameLayout<FName>::Load(StreamingLevels + Idx * TGameLayout<FName>::Size());
        auto Ptr = AlwaysRelevantStreamingLevelActors.Find(StreamingLevel);
        bool bRemove = !Ptr;

        if (Ptr && Ptr->RepList.Num() > 0)
        {
            bool bAllDormant = true;

            for (auto Actor : Ptr->RepList)
            {
                if (!ActorInfoMap.FindOrAdd(Actor).bDormantOnConnection)
                {
                    bAllDormant = false;
                    break;
                }
            }

            if (bAllDormant)
                bRemove = true;
            else
                Params.GetOutGatheredReplicationLists()->AddReplicationActorList(*Ptr);
        }

        if (bRemove)
        {
            if (Idx != NumAlwaysRelevantStreamingLevels - 1)
                memcpy(StreamingLevels + Idx * TGameLayout<FName>::Size(), StreamingLevels + (NumAlwaysRelevantStreamingLevels - 1) * TGameLayout<FName>::Size(), TGameLayout<FName>::Size());

            NumAlwaysRelevantStreamingLevels--;
        }
    }

    auto World = GetGraphGlobals().IsValid() ? GetGraphGlobals()->World : nullptr;

    if (!World)
        return;

    auto GameState = Cast<AFortGameStateAthena>(World->GetGameState());

    if (!GameState)
        return;

    if (bIsReplay)
        ReplicationActorList.ConditionalAdd((AActor*)GameState->GetBroadcastSpectatorInfo());

    auto SafeZoneIndicator = GameState->GetSafeZoneIndicator();
    auto FutureReplicator = SafeZoneIndicator ? SafeZoneIndicator->GetFutureReplicator() : nullptr;

    if (!FutureReplicator)
        return;

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        auto PlayerController = Cast<AFortPlayerController>(Params.GetViewer(Index).InViewer);
        auto WhitelistedPlayers = AFortSafeZoneIndicatorFuture::GetWhitelistedPlayers();

        if (PlayerController && WhitelistedPlayers->Contains(FObjectKey(PlayerController)))
            ReplicationActorList.ConditionalAdd((AActor*)FutureReplicator);
    }
}

UClass* UFortReplicationGraphNode_PlayerStateFrequencyLimiter::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_PlayerStateFrequencyLimiter"));

    return Class;
}

int32& UFortReplicationGraphNode_PlayerStateFrequencyLimiter::GetCurrentFrameListIndex()
{
    static auto CurrentFrameListIndex = Anchor::Offset(TEXT("UFortReplicationGraphNode_PlayerStateFrequencyLimiter::CurrentFrameListIndex"))
        .String(TEXT("UFortReplicationGraphNode_PlayerStateFrequencyLimiter_GlobalPrepareForReplication"))
        .Start()
        .Find("mov ecx, [rcx+0x*]")
        .Disp();

    return *(int32*)((uint8*)this + CurrentFrameListIndex);
}

int32& UFortReplicationGraphNode_PlayerStateFrequencyLimiter::GetMaxStatesToReplicatePerFrame()
{
    return *(&GetCurrentFrameListIndex() + 1);
}

TBitArray<>& UFortReplicationGraphNode_PlayerStateFrequencyLimiter::GetActiveLists()
{
    return *(TBitArray<>*)(&GetMaxStatesToReplicatePerFrame() + 1);
}

TArray<FActorRepListRefView>& UFortReplicationGraphNode_PlayerStateFrequencyLimiter::GetPerFramePlayerStateLists()
{
    return *((TArray<FActorRepListRefView>*)&GetCurrentFrameListIndex() - 1);
}

TMap<APlayerState*, int32>& UFortReplicationGraphNode_PlayerStateFrequencyLimiter::GetPlayerStateStoredListIndexMap()
{
    return *((TMap<APlayerState*, int32>*)&GetPerFramePlayerStateLists() - 1);
}

void UFortReplicationGraphNode_PlayerStateFrequencyLimiter::InitNode(UWorld* World)
{
    static auto MaxExpectedPlayerStates = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.MaxExpectedPlayerStates"));

    const int32 NumLists = MaxExpectedPlayerStates->GetInt() / GetMaxStatesToReplicatePerFrame();

    GetPerFramePlayerStateLists().Reserve(NumLists);
    GetActiveLists().Reserve(NumLists);

    TArray<AActor*> PlayerStates;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerState::StaticClass(), PlayerStates);

    for (auto PlayerState : PlayerStates)
    {
        if (IsActorValidForReplicationGather(PlayerState))
            OnPlayerStateAdded((APlayerState*)PlayerState);
    }

    AFortGameStateAthena::GetNotifyOnPlayerStateAdded().AddUObject(this, &UFortReplicationGraphNode_PlayerStateFrequencyLimiter::OnPlayerStateAdded);
    AFortGameStateAthena::GetNotifyOnPlayerStateRemoved().AddUObject(this, &UFortReplicationGraphNode_PlayerStateFrequencyLimiter::OnPlayerStateRemoved);
}

void UFortReplicationGraphNode_PlayerStateFrequencyLimiter::OnPlayerStateAdded(APlayerState* AddedPlayerState)
{
    if (!IsActorValidForReplicationGather(AddedPlayerState))
        return;

    auto& PlayerStateStoredListIndexMap = GetPlayerStateStoredListIndexMap();

    if (PlayerStateStoredListIndexMap.Find(AddedPlayerState))
        return;

    auto& PerFramePlayerStateLists = GetPerFramePlayerStateLists();
    bool bNeedsNewList = true;

    for (int32 ListIdx = 0; ListIdx < PerFramePlayerStateLists.Num(); ++ListIdx)
    {
        auto& List = PerFramePlayerStateLists[ListIdx];

        if (List.RepList.Num() < GetMaxStatesToReplicatePerFrame())
        {
            PlayerStateStoredListIndexMap.Add(AddedPlayerState, ListIdx);
            List.RepList.Add(AddedPlayerState);
            GetActiveLists().SetBit(ListIdx, true);
            bNeedsNewList = false;
            break;
        }
    }

    if (bNeedsNewList)
    {
        auto& NewList = PerFramePlayerStateLists[PerFramePlayerStateLists.Add(FActorRepListRefView())];

        GetActiveLists().Add(true);
        PlayerStateStoredListIndexMap.Add(AddedPlayerState, PerFramePlayerStateLists.Num() - 1);

        NewList.RepList.Reset(GetMaxStatesToReplicatePerFrame());
        NewList.RepList.Add(AddedPlayerState);
    }
}

void UFortReplicationGraphNode_PlayerStateFrequencyLimiter::OnPlayerStateRemoved(APlayerState* RemovedPlayerState)
{
    auto& PlayerStateStoredListIndexMap = GetPlayerStateStoredListIndexMap();
    auto ListIdxPtr = PlayerStateStoredListIndexMap.Find(RemovedPlayerState);

    if (!ListIdxPtr)
        return;

    const int32 ListIdx = *ListIdxPtr;
    PlayerStateStoredListIndexMap.Remove(RemovedPlayerState);

    if (ListIdx == -1)
        return;

    auto& List = GetPerFramePlayerStateLists()[ListIdx];
    List.RepList.RemoveSingleSwap(RemovedPlayerState, true);

    GetActiveLists().SetBit(ListIdx, List.RepList.Num() > 0);
}

UClass* UFortReplicationGraphNode_DynamicSpatialFrequency::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_DynamicSpatialFrequency"));

    return Class;
}

TClassMap<bool>& UFortReplicationGraphNode_DynamicSpatialFrequency::GetAIClasses()
{
    static auto AIClasses = Anchor::Data<TClassMap<bool>*>(TEXT("UFortReplicationGraphNode_DynamicSpatialFrequency::AIClasses"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"))
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *AIClasses;
}

UReplicationGraphNode_DynamicSpatialFrequency::FSettings* UFortReplicationGraphNode_DynamicSpatialFrequency::GetNodeSettings()
{
    static auto NodeSettings = Anchor::Data<FSettings*>(TEXT("UFortReplicationGraphNode_DynamicSpatialFrequency::NodeSettings"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"))
        .Find("call 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Get();

    return NodeSettings;
}

void UFortReplicationGraphNode_DynamicSpatialFrequency::SetupClassToNodeMap()
{
    auto& DynamicTypeToNodeMap = GetDynamicTypeToNodeMap();

    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::WorldStatic, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::WorldDynamic, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::Pawn, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
}

EFortDynamicNodeType& UFortReplicationGraphNode_DynamicSpatialFrequency::GetCurrentDynamicNodeType()
{
    static auto CurrentDynamicNodeType = Anchor::Offset(TEXT("UFortReplicationGraphNode_DynamicSpatialFrequency::CurrentDynamicNodeType"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"))
        .Find("call 0x*")
        .Target()
        .Find("mov [rcx+0x*], bpl")
        .Disp();

    return *(EFortDynamicNodeType*)((uint8*)this + CurrentDynamicNodeType);
}

TMap<EFortDynamicNodeType, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>>& UFortReplicationGraphNode_DynamicSpatialFrequency::GetDynamicTypeToNodeMap()
{
    return *(TMap<EFortDynamicNodeType, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>>*)((uint8*)&GetCurrentDynamicNodeType() + sizeof(void*));
}

void UFortReplicationGraphNode_DynamicSpatialFrequency::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UFortReplicationGraphNode_DynamicSpatialFrequency, GatherActorListsForConnection, Anchor::Hook(TEXT("UFortReplicationGraphNode_DynamicSpatialFrequency::GatherActors"), &UFortReplicationGraphNode_DynamicSpatialFrequency::GatherActors)
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"), "ret*")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"), 0x30)
        .Optional()
        .InChildVTables(UFortReplicationGraphNode_DynamicSpatialFrequency::StaticClass()), Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_DynamicSpatialFrequency"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraphNode_DynamicSpatialFrequency::StaticClass()));

    for (const auto& Pair : GetDynamicTypeToNodeMap())
    {
        if (UReplicationGraphNode* Node = Pair.Value.Get())
            Node->GatherActorListsForConnection(Params);
    }

    UReplicationGraphNode_DynamicSpatialFrequency::GatherActorListsForConnection(Params);
}

void UFortReplicationGraphNode_DynamicSpatialFrequency::GatherActors(const FActorRepListRefView& RepList, FGlobalActorReplicationInfoMap& GlobalMap, FPerConnectionActorInfoMap& ConnectionMap, const FConnectionGatherActorListParameters& Params, UNetConnection* NetConnection)
{
    auto RepGraph = GetGraphGlobals()->ReplicationGraph;
    auto& MySettings = GetSettings();
    const uint32 FrameNum = Params.GetReplicationFrameNum();
    TArray<AActor*> TetherPawns;

    if (GetCurrentDynamicNodeType() == EFortDynamicNodeType::WorldStatic)
    {
        for (const FNetViewer& CurViewer : Params.GetViewers())
        {
            AActor* TetherPawn = nullptr;

            if (auto PlayerPawn = Cast<AFortPlayerPawn>(CurViewer.ViewTarget))
                TetherPawn = PlayerPawn->GetTetherPawn();
            else if (auto AIPawn = Cast<AFortAIPawn>(CurViewer.ViewTarget))
                TetherPawn = AIPawn->GetTetheredFollower();

            if (TetherPawn)
                TetherPawns.AddUnique(TetherPawn);
        }
    }

    for (AActor* Actor : RepList.RepList)
    {
        bool bIsAViewTarget = false;

        for (const FNetViewer& CurViewer : Params.GetViewers())
        {
            if (Actor == CurViewer.ViewTarget)
            {
                bIsAViewTarget = true;
                break;
            }
        }

        if (TetherPawns.Contains(Actor) || bIsAViewTarget)
            continue;

        CalcFrequencyForActor(Actor, RepGraph, NetConnection, GlobalMap.Get(Actor), ConnectionMap, MySettings, Params.GetViewers(), FrameNum, INDEX_NONE);
    }
}

UClass* UFortReplicationGraphNode_LiveSpectator::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_LiveSpectator"));

    return Class;
}

FActorRepListRefView& UFortReplicationGraphNode_LiveSpectator::GetBroadcasterRelevancyActorList()
{
    static auto BroadcasterRelevancyActorList = Anchor::Offset(TEXT("UFortReplicationGraphNode_LiveSpectator::BroadcasterRelevancyActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x38)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_LiveSpectator"))
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + BroadcasterRelevancyActorList);
}

TArray<FFortAlwaysRelevantActorInfo>& UFortReplicationGraphNode_LiveSpectator::GetPastViewedTargets()
{
    return *(TArray<FFortAlwaysRelevantActorInfo>*)(&GetBroadcasterRelevancyActorList() + 1);
}

void UFortReplicationGraphNode_LiveSpectator::GatherBroadcasterActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    static auto RelevancyClassesFrameSpread = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.Broadcast.RelevancyClassesFrameSpread"));

    auto& BroadcasterRelevancyActorList = GetBroadcasterRelevancyActorList();
    const int32 NumActors = BroadcasterRelevancyActorList.RepList.Num();

    for (int32 Idx = Params.GetReplicationFrameNum() % (uint32)RelevancyClassesFrameSpread->GetInt(); Idx < NumActors; Idx += RelevancyClassesFrameSpread->GetInt())
        GetReplicationActorList().ConditionalAdd(BroadcasterRelevancyActorList.RepList[Idx]);
}

void UFortReplicationGraphNode_LiveSpectator::GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params)
{
    DETOUR(UFortReplicationGraphNode_LiveSpectator, GatherActorListsForConnection, Anchor::Hook()
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_LiveSpectator"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraphNode_LiveSpectator::StaticClass()));

    auto& ReplicationActorList = GetReplicationActorList();
    ReplicationActorList.RepList.Reset();

    bool bIsBroadcaster = false;
    TArray<AActor*> ProcessedViewTargets;

    for (int32 Index = 0; Index < Params.GetNumViewers(); Index++)
    {
        const FNetViewer& CurViewer = Params.GetViewer(Index);
        auto SpectatorController = Cast<AFortLiveSpectatorController>(CurViewer.InViewer);

        if (!SpectatorController)
            continue;

        bIsBroadcaster = SpectatorController->IsA(AFortLiveBroadcastController::StaticClass());

        ReplicationActorList.ConditionalAdd((AActor*)SpectatorController->GetFollowedPlayerRemoteClientInfo());

        if (auto FollowedPlayerState = SpectatorController->GetFollowedPlayerState())
            ReplicationActorList.ConditionalAdd((AActor*)FollowedPlayerState->GetPlayerTeamPrivate());

        TArray<AFortBroadcastRemoteClientInfo*> RemoteClientInfos;
        SpectatorController->GetRemoteClientInfos(RemoteClientInfos);

        for (auto RemoteClientInfo : RemoteClientInfos)
            ReplicationActorList.ConditionalAdd((AActor*)RemoteClientInfo);

        ReplicationActorList.ConditionalAdd((AActor*)SpectatorController->GetViewTargetInventory());

        auto& PastViewedTargets = GetPastViewedTargets();
        auto LastData = PastViewedTargets.FindByKey(CurViewer.Connection);

        if (!LastData)
        {
            FFortAlwaysRelevantActorInfo NewActorInfo = { CurViewer.Connection, nullptr, nullptr };
            LastData = &PastViewedTargets[PastViewedTargets.Add(NewActorInfo)];
        }

        AActor* ViewTarget = SpectatorController->GetFollowedPlayerPawn(true);

        if (Cast<AFortPlayerPawn>(ViewTarget))
            LastData->LastPawn = ViewTarget;
        else
            ViewTarget = LastData->LastPawn;

        if (!ViewTarget || !IsActorValidForReplication(ViewTarget) || ProcessedViewTargets.Contains(ViewTarget))
            continue;

        Params.GetConnectionManager()->GetActorInfoMap().FindOrAdd(ViewTarget).NextReplicationFrameNum = Params.GetReplicationFrameNum();
        ReplicationActorList.ConditionalAdd(ViewTarget);
        ProcessedViewTargets.Add(ViewTarget);
    }

    GetPastViewedTargets().RemoveAll([](const FFortAlwaysRelevantActorInfo& RelActorInfo)
    {
        return RelActorInfo.Connection == nullptr;
    });

    if (bIsBroadcaster)
        GatherBroadcasterActorListsForConnection(Params);

    Params.GetOutGatheredReplicationLists()->AddReplicationActorList(ReplicationActorList);
}

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
