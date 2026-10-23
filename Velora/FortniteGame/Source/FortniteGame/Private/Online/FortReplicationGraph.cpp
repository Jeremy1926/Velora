#include "pch.h"
#include "FortReplicationGraph.h"
#include "FortTeamActorInterface.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/BitArray.h"
#include "GameFramework/PlayerState.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/FortAIPawn.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortLiveBroadcastController.h"
#include "Player/FortLiveSpectatorController.h"
#include "Player/FortPlayerState.h"
#include "Player/FortTeamHealthInfo.h"
#include "HAL/IConsoleManager.h"
#include "HAL/UnrealMemory.h"
#include "Templates/Casts.h"
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
