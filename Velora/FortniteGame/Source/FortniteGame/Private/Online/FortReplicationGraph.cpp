#include "pch.h"
#include "FortReplicationGraph.h"
#include "FortTeamActorInterface.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Athena/Modifiers/SpecialRelevancyNode.h"
#include "Athena/Vehicles/FortAthenaExitCraft.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Building/BuildingActor.h"
#include "Building/BuildingContainer.h"
#include "Building/BuildingGameplayActorSpawnChip.h"
#include "Components/FortPhysicsObjectComponent.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Containers/BitArray.h"
#include "GameFramework/PlayerState.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "FortGameInstance.h"
#include "FortGameMode.h"
#include "FortRuntimeOptions.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/ReplicationDriver.h"
#include "UpdateManager.h"
#include "Pawns/FortAIPawn.h"
#include "Pawns/FortPawn.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortLiveBroadcastController.h"
#include "Player/FortLiveSpectatorController.h"
#include "Player/FortPlayerController.h"
#include "Player/FortPlayerState.h"
#include "Player/FortTeamHealthInfo.h"
#include "Player/FortTeamPrivateInfo.h"
#include "UObject/ObjectKey.h"
#include "HAL/IConsoleManager.h"
#include "HAL/UnrealMemory.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "Templates/Function.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/SparseDelegate.h"
#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Stack.h"
#include "UObject/UObjectArray.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Weapons/FortWeapon.h"
#include "SimObject.h"
#include "Scanner.h"
#include "Detour.h"

UReplicationDriver* ConditionalCreateReplicationDriver(UNetDriver* ForNetDriver, UWorld* World)
{
    if (ForNetDriver->GetNetDriverName() != NAME_GameNetDriver)
        return nullptr;

    auto FortGameMode = Cast<AFortGameMode>(World->GetAuthGameMode());

    if (!FortGameMode || !FortGameMode->GetEnableReplicationGraph())
        return nullptr;

    if (UFortRuntimeOptions::Get().GetDisableReplicationGraph())
        return nullptr;

    return NewObject<UFortReplicationGraph>(GetTransientPackage(), UFortReplicationGraph::StaticClass());
}

void ForceRecreateReplicationGraph(UNetDriver* NetDriver)
{
    NetDriver->SetReplicationDriver(nullptr);
    NetDriver->SetReplicationDriver(ConditionalCreateReplicationDriver(NetDriver, NetDriver->GetWorld()));
}

void FortRepGraphOnHotFixApplied(EUpdateCompletionStatus Status)
{
    if (Status != EUpdateCompletionStatus::UpdateSuccess)
        return;

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        auto ObjectItem = GUObjectArray.IndexToValidObject(Index, true);
        auto NetDriver = ObjectItem ? Cast<UNetDriver>((UObject*)ObjectItem->Object) : nullptr;

        if (!NetDriver || (NetDriver->GetFlags() & (RF_ClassDefaultObject | 0x60000000)))
            continue;

        if (NetDriver->GetNetDriverName() != NAME_GameNetDriver || NetDriver->GetClientConnections().Num() > 0 || !NetDriver->GetWorld())
            continue;

        ForceRecreateReplicationGraph(NetDriver);
    }
}

void FortRepGraphOnPlaylistDataLoaded(AFortGameModeAthena* GameMode)
{
    auto NetDriver = GameMode->GetNetDriver();

    if (!NetDriver)
        return;

    if (auto FortReplicationGraph = Cast<UFortReplicationGraph>(NetDriver->GetReplicationDriver()))
        FortReplicationGraph->InitForPlaylist(GameMode);
}

void BindCreateReplicationDriverDelegate()
{
    if (!UFortReplicationGraph::StaticClass() || UReplicationDriver::CreateReplicationDriverDelegate().IsBound())
        return;

    UReplicationDriver::CreateReplicationDriverDelegate().BindLambda([](UNetDriver* ForNetDriver, const FURL& URL, UWorld* World) -> UReplicationDriver*
    {
        if (World)
        {
            if (auto UpdateManager = ((UFortGameInstance*)World->GetGameInstance())->GetUpdateManager())
            {
                [[maybe_unused]] static auto UpdateCheckCompleteHandle = UpdateManager->OnUpdateCheckComplete().AddStatic(&FortRepGraphOnHotFixApplied);
            }
        }

        [[maybe_unused]] static auto NotifyPlaylistDataLoadedHandle = AFortGameModeAthena::GetNotifyPlaylistDataLoaded().AddStatic(&FortRepGraphOnPlaylistDataLoaded);

        return ConditionalCreateReplicationDriver(ForNetDriver, World);
    });
}

UScriptStruct* FRepGraphActorSettingsBase::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.RepGraphActorSettingsBase"));

    return Struct;
}

bool FRepGraphActorSettingsBase::GetAddClassRepInfoToMap() const
{
    static auto bAddClassRepInfoToMap = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bAddClassRepInfoToMap")));

    return bAddClassRepInfoToMap->GetPropertyValue_InContainer(this);
}

bool FRepGraphActorSettingsBase::GetUseCustomClassRepInfo() const
{
    static auto bUseCustomClassRepInfo = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bUseCustomClassRepInfo")));

    return bUseCustomClassRepInfo->GetPropertyValue_InContainer(this);
}

bool FRepGraphActorSettingsBase::GetAddToRPC_Multicast_OpenChannelForClassMap() const
{
    static auto bAddToRPC_Multicast_OpenChannelForClassMap = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bAddToRPC_Multicast_OpenChannelForClassMap")));

    return bAddToRPC_Multicast_OpenChannelForClassMap->GetPropertyValue_InContainer(this);
}

bool FRepGraphActorSettingsBase::GetRPC_Multicast_OpenChannelForClass() const
{
    static auto bRPC_Multicast_OpenChannelForClass = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bRPC_Multicast_OpenChannelForClass")));

    return bRPC_Multicast_OpenChannelForClass->GetPropertyValue_InContainer(this);
}

EClassRepNodeMapping FRepGraphActorSettingsBase::GetClassNodeMapping() const
{
    static auto ClassNodeMapping = StaticStruct()->FindPropertyByName(FName(TEXT("ClassNodeMapping")));

    return (EClassRepNodeMapping)*ClassNodeMapping->ContainerPtrToValuePtr<uint8>(this);
}

FClassReplicationInfo& FRepGraphActorSettingsBase::GetClassRepInfo()
{
    static auto ClassRepInfo = StaticStruct()->FindPropertyByName(FName(TEXT("ClassRepInfo")));

    return *ClassRepInfo->ContainerPtrToValuePtr<FClassReplicationInfo>(this);
}

UScriptStruct* FRepGraphActorClassSettings::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.RepGraphActorClassSettings"));

    return Struct;
}

int32 FRepGraphActorClassSettings::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FName FRepGraphActorClassSettings::GetActorClassName() const
{
    static auto ActorClassName = StaticStruct()->FindPropertyByName(FName(TEXT("ActorClassName")));

    return ActorClassName->GetPropertyValue_InContainer<FName>(this);
}

UClass* FRepGraphActorClassSettings::GetStaticActorClass() const
{
    auto ActorClassNameString = GetActorClassName().ToString();

    if (ActorClassNameString.StartsWith(TEXT("/Script/")))
        return (UClass*)StaticFindObject(UClass::StaticClass(), (UObject*)-1, *ActorClassNameString, true);

    return LoadObject<UClass>(nullptr, *ActorClassNameString);
}

UClass* UGlobalRepGraphActorClassSettings::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.GlobalRepGraphActorClassSettings"));

    return Class;
}

TArray<FRepGraphActorClassSettings>& UGlobalRepGraphActorClassSettings::GetClassSettings()
{
    static auto ClassSettings = StaticClass()->FindPropertyByName(FName(TEXT("ClassSettings")));

    return *ClassSettings->ContainerPtrToValuePtr<TArray<FRepGraphActorClassSettings>>(this);
}

UClass* UFortReplicationGraphConnection::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphConnection"));

    return Class;
}

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

TMap<FObjectKey, UFortReplicationGraphNode_AlwaysRelevantForTeam*>& UFortReplicationGraph::GetFortTeamPrivateInfoMap()
{
    return *(TMap<FObjectKey, UFortReplicationGraphNode_AlwaysRelevantForTeam*>*)(&GetClassRepNodePolicies() + 1);
}

TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>& UFortReplicationGraph::GetSquadMap()
{
    return *(TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>*)((uint8*)(&GetClassRepNodePolicies() + 1) + 2 * sizeof(TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>));
}

bool& UFortReplicationGraph::GetSpecialRelevancyEnabled()
{
    return *((bool*)(&GetSquadMap() + 1) + 1);
}

TArray<UClass*>& UFortReplicationGraph::GetExplicitlySetClasses()
{
    return *((TArray<UClass*>*)&GetGlobalActorClassSettings() - 1);
}

UGlobalRepGraphActorClassSettings*& UFortReplicationGraph::GetGlobalActorClassSettings()
{
    static auto GlobalActorClassSettings = Anchor::Offset(TEXT("UFortReplicationGraph::GlobalActorClassSettings"))
        .String(TEXT("GlobalRepGraphActorClassSettings"))
        .Start()
        .Caller()
        .CallSite()
        .Find("mov [*+0x*], rax")
        .Disp();

    return *(UGlobalRepGraphActorClassSettings**)((uint8*)this + GlobalActorClassSettings);
}

UReplicationGraphNode_ActorList*& UFortReplicationGraph::GetAlwaysRelevantNode()
{
    static auto AlwaysRelevantNode = StaticClass()->FindPropertyByName(FName(TEXT("AlwaysRelevantNode")));

    return *AlwaysRelevantNode->ContainerPtrToValuePtr<UReplicationGraphNode_ActorList*>(this);
}

UReplicationGraphNode*& UFortReplicationGraph::GetLiveSpectatorRelevancyNode()
{
    static auto LiveSpectatorRelevancyNode = StaticClass()->FindPropertyByName(FName(TEXT("LiveSpectatorRelevancyNode")));

    return *LiveSpectatorRelevancyNode->ContainerPtrToValuePtr<UReplicationGraphNode*>(this);
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

void UFortReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
    DETOUR(UFortReplicationGraph, RouteAddNetworkActorToNodes, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x48)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    const auto Policy = GetMappingPolicy(ActorInfo.Class, ActorInfo.Actor);

    if (Policy >= EClassRepNodeMapping::Spatialize_Static)
    {
        auto RootGridNode = GetRootGridNode();

        if (Policy == EClassRepNodeMapping::Spatialize_Static)
            RootGridNode->AddActor_Static(ActorInfo, GlobalInfo);
        else if (Policy == EClassRepNodeMapping::Spatialize_Dynamic)
            RootGridNode->AddActor_Dynamic(ActorInfo, GlobalInfo);
        else if (Policy == EClassRepNodeMapping::Spatialize_Dormancy)
            RootGridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
    }
    else if (Policy == EClassRepNodeMapping::RelevantAllConnections)
    {
        AddRelevantToAllConnections(ActorInfo, GlobalInfo);
    }
    else if (Policy == EClassRepNodeMapping::RelevantAllInsideFortVolume)
    {
        AddRelevantToAllInsideFortVolume(ActorInfo, GlobalInfo);
    }
    else if (Policy == EClassRepNodeMapping::Custom)
    {
        auto NodePtr = GetClassToNodetoRouteTo().Find(ActorInfo.Class);

        if (NodePtr && *NodePtr)
            (*NodePtr)->NotifyAddNetworkActor(ActorInfo);
    }

    if (auto LiveSpectatorRelevancyNode = GetLiveSpectatorRelevancyNode())
        LiveSpectatorRelevancyNode->NotifyAddNetworkActor(ActorInfo);
}

void UFortReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
    DETOUR(UFortReplicationGraph, RouteRemoveNetworkActorToNodes, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x50)
        .HasVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "cmp eax, 0x4; jnl 0x*")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    const auto Policy = GetMappingPolicy(ActorInfo.Class, ActorInfo.Actor);

    if (Policy == EClassRepNodeMapping::Spatialize_Static)
        GetRootGridNode()->RemoveActor_Static(ActorInfo);
    else if (Policy == EClassRepNodeMapping::Spatialize_Dynamic)
        GetRootGridNode()->RemoveActor_Dynamic(ActorInfo);
    else if (Policy == EClassRepNodeMapping::Spatialize_Dormancy)
        GetRootGridNode()->RemoveActor_Dormancy(ActorInfo);

    Original(ActorInfo);
}

void UFortReplicationGraph::InitGlobalActorClassSettings()
{
    DETOUR(UFortReplicationGraph, InitGlobalActorClassSettings, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x90)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "jmp 0x*")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    static auto TargetKBitsSecFastSharedPath = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.TargetKBitsSecFastSharedPath"));
    static auto FastSharedPathCullDistPct = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.FastSharedPathCullDistPct"));
    static auto MinigamePriorityBias = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.MinigamePriorityBias"));
    static auto PropertyOverridePriorityBias = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.PropertyOverridePriorityBias"));
    static auto DestructInfoMaxDist = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DestructInfo.MaxDist"));

    Original();

    UFortReplicationGraphNode_DynamicSpatialFrequency::GetAIClasses().Reset();

    GetReplicationConnectionManagerClass() = UFortReplicationGraphConnection::StaticClass();

    if (UFortRuntimeOptions::Get().GetBRServerMaxTickRate() > 0.f)
        GetNetDriver()->SetNetServerMaxTickRate((int32)UFortRuntimeOptions::Get().GetBRServerMaxTickRate());

    GetGlobalActorReplicationInfoMap().SetInitClassInfoFunc([this](UClass* Class, FClassReplicationInfo& ClassInfo) -> bool
    {
        RegisterClassRepNodeMapping(Class);

        return ConditionalInitClassReplicationInfo(Class, ClassInfo);
    });

    GetClassRepNodePolicies().InitNewElement = [this](UClass* ReplicatedClass, EClassRepNodeMapping& OutMapping) -> bool
    {
        OutMapping = GetClassNodeMapping(ReplicatedClass);

        return true;
    };

    auto& ClassSettings = GetGlobalActorClassSettings()->GetClassSettings();

    for (auto& ActorClassSettings : ClassSettings)
    {
        if (!ActorClassSettings.GetAddClassRepInfoToMap())
            continue;

        if (auto StaticActorClass = ActorClassSettings.GetStaticActorClass())
            AddClassRepInfo(StaticActorClass, ActorClassSettings.GetClassNodeMapping());
    }

    AddClassRepInfo(AFortSafeZoneIndicatorFuture::StaticClass(), EClassRepNodeMapping::NotRouted);

    TArray<UClass*> AllReplicatedClasses;

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        auto ObjectItem = GUObjectArray.IndexToValidObject(Index, true);
        auto Class = ObjectItem ? Cast<UClass>((UObject*)ObjectItem->Object) : nullptr;

        if (!Class || (Class->GetFlags() & RF_ClassDefaultObject))
            continue;

        auto ActorCDO = Cast<AActor>(Class->GetDefaultObject(true));

        if (!ActorCDO || !ActorCDO->GetIsReplicated())
            continue;

        auto ClassName = Class->GetFName().ToString();

        if (ClassName.StartsWith(TEXT("SKEL_")) || ClassName.StartsWith(TEXT("REINST_")))
            continue;

        AllReplicatedClasses.Add(Class);
        RegisterClassRepNodeMapping(Class);
    }

    GetExplicitlySetClasses().Reset();

    auto SetClassInfo = [&](UClass* Class, const FClassReplicationInfo& Info)
    {
        GetGlobalActorReplicationInfoMap().SetClassInfo(Class, Info);
        GetExplicitlySetClasses().Add(Class);
    };

    FClassReplicationInfo PawnClassRepInfo;
    PawnClassRepInfo.SetCullDistanceSquared(15000.f * 15000.f);
    SetClassInfo(APawn::StaticClass(), PawnClassRepInfo);

    PawnClassRepInfo.FastSharedReplicationFunc = [](AActor* Actor) -> bool
    {
        auto FortPawn = Cast<AFortPawn>(Actor);

        return FortPawn ? FortPawn->UpdateSharedReplication() : false;
    };
    PawnClassRepInfo.SetFastSharedReplicationFuncName(FName(TEXT("FastSharedReplication")));

    GetFastSharedPathConstants().MaxBitsPerFrame = (int32)((float)TargetKBitsSecFastSharedPath->GetInt() / (float)GetNetDriver()->GetNetServerMaxTickRate());
    GetFastSharedPathConstants().DistanceRequirementPct = FastSharedPathCullDistPct->GetFloat();

    SetClassInfo(AFortPlayerPawnAthena::StaticClass(), PawnClassRepInfo);
    SetClassInfo(AFortAIPawn::StaticClass(), PawnClassRepInfo);

    for (auto& ActorClassSettings : ClassSettings)
    {
        if (!ActorClassSettings.GetUseCustomClassRepInfo())
            continue;

        auto ActorClassName = ActorClassSettings.GetActorClassName().ToString();

        if (ActorClassName == TEXT("/Script/FortniteGame.FortMinigame"))
            ActorClassSettings.GetClassRepInfo().AccumulatedNetPriorityBias = MinigamePriorityBias->GetFloat();
        else if (ActorClassName == TEXT("/Script/FortniteGame.FortPropertyOverrideReplShared"))
            ActorClassSettings.GetClassRepInfo().AccumulatedNetPriorityBias = PropertyOverridePriorityBias->GetFloat();

        if (auto StaticActorClass = ActorClassSettings.GetStaticActorClass())
            SetClassInfo(StaticActorClass, ActorClassSettings.GetClassRepInfo());
    }

    UReplicationGraphNode_ActorListFrequencyBuckets::GetDefaultSettings().ListSize = 12;

    for (auto ReplicatedClass : AllReplicatedClasses)
    {
        FClassReplicationInfo ClassInfo;

        if (ConditionalInitClassReplicationInfo(ReplicatedClass, ClassInfo))
            GetGlobalActorReplicationInfoMap().SetClassInfo(ReplicatedClass, ClassInfo);
    }

    GetDestructInfoMaxDistanceSquared() = DestructInfoMaxDist->GetFloat() * DestructInfoMaxDist->GetFloat();

    AFortGameModeAthena::GetNotifyPlayerStateSquadAssignment().AddUObject(this, &UFortReplicationGraph::OnPlayerStateSquadAssign);
    AFortGameModeAthena::GetNotifyNewAIClass().AddUObject(this, &UFortReplicationGraph::OnAIClassAdditionRequest);
    AFortTeamPrivateInfo::GetNotifyPlayerAdded().AddUObject(this, &UFortReplicationGraph::OnPlayerAddedTeam);
    AFortTeamPrivateInfo::GetNotifyPlayerRemoved().AddUObject(this, &UFortReplicationGraph::OnPlayerRemovedTeam);
    AFortPawn::GetNotifyWeaponChange().AddUObject(this, &UFortReplicationGraph::OnPawnWeaponChange);
    AFortPlayerPawn::GetVehicleStateChange().AddUObject(this, &UFortReplicationGraph::OnVehicleChanged);
    AFortPlayerPawn::GetNotifyPetStateChange().AddUObject(this, &UFortReplicationGraph::OnPetStateChange);
    AFortPlayerPawn::GetNotifyBalloonStateChange().AddUObject(this, &UFortReplicationGraph::OnBalloonStateChange);
    AFortPlayerPawn::GetNotifyPossessedPropStateChange().AddUObject(this, &UFortReplicationGraph::OnPossessedPropStateChange);
    AFortPawn::GetNotifyPawnNetCullDistanceChanged().AddUObject(this, &UFortReplicationGraph::OnPawnCullDistSqChange);
    AFortAthenaExitCraft::GetOnExitVehicleCullDistChange().AddUObject(this, &UFortReplicationGraph::OnExitVehicleCullDistChange);
    AFortPickup::GetNotifyPickupCullDistanceChange().AddUObject(this, &UFortReplicationGraph::OnPickupCullDistanceChanged);
    ABuildingActor::GetNotifyBuildingNetCullDistChange().AddUObject(this, &UFortReplicationGraph::OnBuildingDistSqChange);
    AFortAthenaVehicle::GetNotifyNetCullDistanceChange().AddUObject(this, &UFortReplicationGraph::OnActorNetCullDistanceChange);
    ASimObject::GetNotifyNetCullDistanceChange().AddUObject(this, &UFortReplicationGraph::OnActorNetCullDistanceChange);
    AFortGameModeAthena::GetNotifyLiveSpectatorJoined().AddUObject(this, &UFortReplicationGraph::OnLiveSpectatorJoined);
    AFortPlayerStateAthena::GetNotifyRebootCardSpawned().AddUObject(this, &UFortReplicationGraph::OnRebootCardSpawned);
    AFortPlayerStateAthena::GetNotifyRebootCardDestroyed().AddUObject(this, &UFortReplicationGraph::OnRebootCardDestroyed);
    AFortPlayerStateAthena::GetNotifySquadSpecificContainerSpawned().AddUObject(this, &UFortReplicationGraph::OnSquadSpecificContainerSpawned);
    AFortPlayerStateAthena::GetNotifySquadSpecificContainerDestroyed().AddUObject(this, &UFortReplicationGraph::OnSquadSpecificContainerDestroyed);
    FFortPlayerControllerEvents::GetOnPlayerStateChangedEvent().AddUObject(this, &UFortReplicationGraph::OnPlayerControllerStateChanged);
    AFortGameModeAthena::GetNotifyAddReplicatedNonPlayerInventory().AddUObject(this, &UFortReplicationGraph::OnNotifyAddReplicatedNonPlayerInventory);
    AFortGameModeAthena::GetNotifyRemoveReplicatedNonPlayerInventory().AddUObject(this, &UFortReplicationGraph::OnNotifyRemoveReplicatedNonPlayerInventory);
    UFortPhysicsObjectComponent::GetNotifyPhysicsObjectComponentActivated().AddUObject(this, &UFortReplicationGraph::OnPhysicsObjectComponentActivated);
    UFortPhysicsObjectComponent::GetNotifyPhysicsObjectComponentDeactivated().AddUObject(this, &UFortReplicationGraph::OnPhysicsObjectComponentDeactivated);

    static auto HighFrequencyPawnUpdates = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.HighFrequencyPawnUpdates"));

    if (HighFrequencyPawnUpdates->GetInt() > 0)
        AFortPlayerPawn::GetNotifyPawnHighFrequencyUpdateChange().AddUObject(this, &UFortReplicationGraph::OnPlayerPawnHighFrequencyChange);

    static auto OverrideActorDiscoveryBudget = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.OverrideActorDiscoveryBudget"));

    OverrideActorDiscoveryBudget->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&UFortReplicationGraph::OnChangeOverrideActorDiscoveryBudget));

    GetRPC_Multicast_OpenChannelForClass().Reset();

    for (auto& ActorClassSettings : ClassSettings)
    {
        if (!ActorClassSettings.GetAddToRPC_Multicast_OpenChannelForClassMap())
            continue;

        if (auto StaticActorClass = ActorClassSettings.GetStaticActorClass())
            GetRPC_Multicast_OpenChannelForClass().Set(StaticActorClass, ActorClassSettings.GetRPC_Multicast_OpenChannelForClass());
    }
}

void UFortReplicationGraph::InitGlobalGraphNodes()
{
    DETOUR(UFortReplicationGraph, InitGlobalGraphNodes, Anchor::Hook()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x98)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), "ret*")
        .Optional()
        .InChildVTables(UFortReplicationGraph::StaticClass()));

    static auto CellSize = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.CellSize"));
    static auto SpatialBiasX = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.SpatialBiasX"));
    static auto SpatialBiasY = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.SpatialBiasY"));
    static auto DisableSpatialRebuilds = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DisableSpatialrebuilds"));
    static auto MaxZDormantNode = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.MaxZDormantNode"));
    static auto ReplayMaxPlayerStatesPerFrame = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.Replay.MaxPlayerStatesPerFrame"));
    static auto DSFEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DSF.Enabled"));

    auto RootGridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
    GetRootGridNode() = RootGridNode;
    RootGridNode->GetCellSize() = CellSize->GetFloat();
    TLWCGameLayout<float, double>::Store(RootGridNode->GetSpatialBias(), SpatialBiasX->GetFloat());
    TLWCGameLayout<float, double>::Store(RootGridNode->GetSpatialBias() + TLWCGameLayout<float, double>::Size(), SpatialBiasY->GetFloat());

    if (DisableSpatialRebuilds->GetInt())
        RootGridNode->AddToClassRebuildDenyList(AActor::StaticClass());

    AddGlobalGraphNode(RootGridNode);

    UReplicationGraphNode_DormancyNode::GetMaxZForConnection() = MaxZDormantNode->GetFloat();

    auto AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
    GetAlwaysRelevantNode() = AlwaysRelevantNode;
    AddGlobalGraphNode(AlwaysRelevantNode);

    auto World = GetNetDriver()->GetWorld();

    auto PlayerStateLimiterNode = CreateNewNode<UFortReplicationGraphNode_PlayerStateFrequencyLimiter>();
    GetPlayerStateLimiterNode() = PlayerStateLimiterNode;
    PlayerStateLimiterNode->InitNode(World);

    auto ReplayPlayerStateLimiterNode = CreateNewNode<UFortReplicationGraphNode_PlayerStateFrequencyLimiter>();
    GetReplayPlayerStateLimiterNode() = ReplayPlayerStateLimiterNode;
    ReplayPlayerStateLimiterNode->GetMaxStatesToReplicatePerFrame() = ReplayMaxPlayerStatesPerFrame->GetInt();
    ReplayPlayerStateLimiterNode->InitNode(World);

    GetLiveSpectatorRelevancyNode() = CreateNewNode<UFortReplicationGraphNode_LiveSpectator>();

    EnableDynamicSpatialFrequencyNode(DSFEnabled->GetInt() != 0);

    InitForPlaylist(Cast<AFortGameModeAthena>(GetNetDriver()->GetWorld()->GetAuthGameMode()));
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
    NodeSettings[(uint8)EFortDynamicNodeType::AI] = FSettings{ { AIZones.GetData(), AIZones.Num() }, { NonFastSharedZones.GetData(), NonFastSharedZones.Num() }, AIPawnMaxBitsPerFrame, MaxAIPerClient->GetInt() };
    NodeSettings[(uint8)EFortDynamicNodeType::PhysicsObject] = FSettings{ { PhysicsObjectZones.GetData(), PhysicsObjectZones.Num() }, { PhysicsObjectZones.GetData(), PhysicsObjectZones.Num() }, PlayerPawnMaxBitsPerFrame, -1 };
    NodeSettings[(uint8)EFortDynamicNodeType::Vehicle] = FSettings{ { VehicleZones.GetData(), VehicleZones.Num() }, { VehicleZones.GetData(), VehicleZones.Num() }, PlayerPawnMaxBitsPerFrame, -1 };
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
            UFortReplicationGraphNode_DynamicSpatialFrequency::GetNodeSettings()[(uint8)EFortDynamicNodeType::AI].MaxNearestActors = *ServerPerClientMaxAI->ContainerPtrToValuePtr<int32>(Playlist);

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

EClassRepNodeMapping UFortReplicationGraph::GetClassNodeMapping(UClass* Class)
{
    if (!Class)
        return EClassRepNodeMapping::NotRouted;

    if (auto Ptr = GetClassRepNodePolicies().FindWithoutClassRecursion(Class))
        return *Ptr;

    auto ActorCDO = Cast<AActor>(Class->GetDefaultObject(true));

    if (!ActorCDO)
        return EClassRepNodeMapping::NotRouted;

    auto SuperClass = (UClass*)Class->GetSuperStruct();

    if (auto SuperCDO = Cast<AActor>(SuperClass->GetDefaultObject(true)))
    {
        if (SuperCDO->GetIsReplicated() == ActorCDO->GetIsReplicated() && SuperCDO->GetAlwaysRelevant() == ActorCDO->GetAlwaysRelevant() && SuperCDO->GetOnlyRelevantToOwner() == ActorCDO->GetOnlyRelevantToOwner() && SuperCDO->GetNetUseOwnerRelevancy() == ActorCDO->GetNetUseOwnerRelevancy())
            return GetClassNodeMapping(SuperClass);
    }

    if (ActorCDO->GetIsReplicated() && !(ActorCDO->GetAlwaysRelevant() || ActorCDO->GetOnlyRelevantToOwner() || ActorCDO->GetNetUseOwnerRelevancy()))
        return ActorCDO->IsReplicatingMovement() ? EClassRepNodeMapping::Spatialize_Dynamic : EClassRepNodeMapping::Spatialize_Static;

    if (ActorCDO->GetAlwaysRelevant() && !ActorCDO->GetOnlyRelevantToOwner())
        return EClassRepNodeMapping::RelevantAllConnections;

    return EClassRepNodeMapping::NotRouted;
}

void UFortReplicationGraph::RegisterClassRepNodeMapping(UClass* Class)
{
    GetClassRepNodePolicies().Set(Class, GetClassNodeMapping(Class));
}

bool UFortReplicationGraph::ConditionalInitClassReplicationInfo(UClass* ReplicatedClass, FClassReplicationInfo& ClassInfo)
{
    for (auto ExplicitlySetClass : GetExplicitlySetClasses())
    {
        if (ReplicatedClass->IsChildOf(ExplicitlySetClass))
            return false;
    }

    InitClassReplicationInfo(ClassInfo, ReplicatedClass, GetClassRepNodePolicies().GetChecked(ReplicatedClass) >= EClassRepNodeMapping::Spatialize_Static);

    return true;
}

void UFortReplicationGraph::InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool bSpatialize)
{
    auto ActorCDO = Class->GetDefaultObject<AActor>();

    if (bSpatialize)
        Info.SetCullDistanceSquared(ActorCDO->GetNetCullDistanceSquared());

    Info.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(ActorCDO->GetNetUpdateFrequency());
}

EClassRepNodeMapping UFortReplicationGraph::GetMappingPolicy(UClass* Class, AActor* Actor)
{
    auto Policy = GetClassRepNodePolicies().GetChecked(Class);

    if (Policy == EClassRepNodeMapping::Instance_Dynamic)
        Policy = GetInstanceDynamicRepNodeMapping(Actor);

    return Policy;
}

EClassRepNodeMapping UFortReplicationGraph::GetInstanceDynamicRepNodeMapping(AActor* Actor)
{
    static auto GetInstanceDynamicRepNodeMapping = Anchor::Function<EClassRepNodeMapping (*)(UFortReplicationGraph*, AActor*)>(TEXT("UFortReplicationGraph::GetInstanceDynamicRepNodeMapping"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortReplicationGraph"), 0x50)
        .Find("cmp eax, 0x7; jnz 0x*; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetInstanceDynamicRepNodeMapping(this, Actor);
}

void UFortReplicationGraph::AddRelevantToAllConnections(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
    if (ActorInfo.StreamingLevelName == NAME_None)
    {
        GetAlwaysRelevantNode()->NotifyAddNetworkActor(ActorInfo);

        return;
    }

    GetAlwaysRelevantStreamingLevelActors().FindOrAdd(ActorInfo.StreamingLevelName).RepList.Add(ActorInfo.Actor);

    OnAlwaysRelevantStreamingActorAdded(ActorInfo.Actor, GlobalInfo);

    auto& DormancyFlush = GlobalInfo.GetEvents().DormancyFlush;

    if (!DormancyFlush.IsBoundToObject(this))
        DormancyFlush.AddUObject(this, &UFortReplicationGraph::OnAlwaysRelevantStreamingActorAdded);
}

void UFortReplicationGraph::AddRelevantToAllInsideFortVolume(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
    AddRelevantToAllConnections(ActorInfo, GlobalInfo);
}

void UFortReplicationGraph::OnAlwaysRelevantStreamingActorAdded(AActor* Actor, FGlobalActorReplicationInfo& GlobalInfo)
{
    FNewReplicatedActorInfo ActorInfo(Actor);

    if (ActorInfo.StreamingLevelName == NAME_None)
        return;

    for (auto ConnectionManager : GetConnections())
    {
        if (!ConnectionManager)
            continue;

        TSet<FName> AllVisibleLevelNames;
        ConnectionManager->GetClientVisibleLevelNames(AllVisibleLevelNames);

        if (!(FName::IsOutlineNumber() ? ((TSet<FNameEntryId>&)AllVisibleLevelNames).Contains(ActorInfo.StreamingLevelName.GetComparisonIndex()) : AllVisibleLevelNames.Contains(ActorInfo.StreamingLevelName)))
            continue;

        for (auto Node : ConnectionManager->GetConnectionGraphNodes())
        {
            if (auto AlwaysRelevantNode = Cast<UFortReplicationGraphNode_AlwaysRelevant_ForConnection>(Node))
                AlwaysRelevantNode->FlushStreamingAlwaysRelevantList(ActorInfo);
        }
    }
}

void UFortReplicationGraph::OnAIClassAdditionRequest(TSubclassOf<AFortPawn> NewAIClass)
{
    if (NewAIClass && ((UClass*)NewAIClass)->IsChildOf(AFortPawn::StaticClass()))
        UFortReplicationGraphNode_DynamicSpatialFrequency::GetAIClasses().Set(NewAIClass, true);
}

void UFortReplicationGraph::OnLiveSpectatorJoined(AFortLiveSpectatorController* PC)
{
    if (!PC)
        return;

    auto NetConnection = PC->GetNetConnection();

    if (!NetConnection)
        return;

    auto ConnectionManager = FindOrAddConnectionManager(NetConnection);

    if (ConnectionManager && GetLiveSpectatorRelevancyNode())
        AddConnectionGraphNode(GetLiveSpectatorRelevancyNode(), ConnectionManager);
}

UFortReplicationGraphNode_AlwaysRelevantForTeam* UFortReplicationGraph::GetNodeForTeamInfo(AFortTeamPrivateInfo* TeamInfo)
{
    const FObjectKey ObjKey(TeamInfo);
    auto& FortTeamPrivateInfoMap = GetFortTeamPrivateInfoMap();

    if (auto Found = FortTeamPrivateInfoMap.Find(ObjKey))
    {
        if (*Found)
            return *Found;
    }

    auto TeamNode = CreateNewNode<UFortReplicationGraphNode_AlwaysRelevantForTeam>();
    TeamNode->GetFortTeamPrivateInfo() = TeamInfo;
    TeamNode->GetReplicationActorList().RepList.Reset();
    TeamNode->GetReplicationActorList().ConditionalAdd(TeamInfo);
    FortTeamPrivateInfoMap.Add(ObjKey, TeamNode);

    return TeamNode;
}

void UFortReplicationGraph::OnPlayerAddedTeam(AFortTeamPrivateInfo* TeamInfo, AFortPlayerState* PS)
{
    if (auto NetConnection = PS->GetNetConnection())
    {
        auto TeamNode = GetNodeForTeamInfo(TeamInfo);
        AddConnectionGraphNode(TeamNode, FindOrAddConnectionManager(NetConnection));
    }
}

void UFortReplicationGraph::OnPlayerRemovedTeam(AFortTeamPrivateInfo* TeamInfo, AFortPlayerState* PS)
{
    if (auto NetConnection = PS->GetNetConnection())
    {
        auto TeamNode = GetNodeForTeamInfo(TeamInfo);
        RemoveConnectionGraphNode(TeamNode, FindOrAddConnectionManager(NetConnection));
    }
}

UFortReplicationGraphNode_AlwaysRelevantForSquad* UFortReplicationGraph::GetNodeForSquad(uint8 SquadID)
{
    auto& SquadMap = GetSquadMap();

    if (auto Found = SquadMap.Find(SquadID))
    {
        if (*Found)
            return *Found;
    }

    if (GetDisableSquadNodes())
        return nullptr;

    auto SquadNode = CreateNewNode<UFortReplicationGraphNode_AlwaysRelevantForSquad>();
    SquadNode->GetSquadID() = SquadID;
    SquadNode->SetSpecialRelevancyEnabled(GetSpecialRelevancyEnabled());
    SquadMap.Add(SquadID, SquadNode);

    return SquadNode;
}

void UFortReplicationGraph::OnPlayerStateSquadAssign(AFortPlayerStateAthena* PS, uint8 PreviousSquadId)
{
    auto NetConnection = PS->GetNetConnection();

    if (!NetConnection)
        return;

    if (auto PreviousSquadNode = GetNodeForSquad(PreviousSquadId))
    {
        PreviousSquadNode->GetPlayerStates().RemoveSwap(PS);
        RemoveConnectionGraphNode(PreviousSquadNode, FindOrAddConnectionManager(NetConnection));
    }

    if (auto SquadNode = GetNodeForSquad(PS->GetSquadId()))
    {
        SquadNode->GetPlayerStates().AddUnique(PS);
        AddConnectionGraphNode(SquadNode, FindOrAddConnectionManager(NetConnection));
    }
}

void UFortReplicationGraph::OnSquadSpecificContainerSpawned(int32 SquadId, ABuildingContainer* Container)
{
    if (!Container)
        return;

    if (auto SquadNode = GetNodeForSquad((uint8)SquadId))
        SquadNode->GetContainers().AddUnique(Container);
}

void UFortReplicationGraph::OnSquadSpecificContainerDestroyed(int32 SquadId, ABuildingContainer* Container)
{
    if (!Container)
        return;

    if (auto SquadNode = GetNodeForSquad((uint8)SquadId))
        SquadNode->GetContainers().RemoveSwap(Container);
}

void UFortReplicationGraph::OnRebootCardSpawned(AFortPlayerStateAthena* PS, ABuildingGameplayActorSpawnChip* Card)
{
    if (!PS->GetNetConnection())
        return;

    if (auto SquadNode = GetNodeForSquad(Card->GetSquadId()))
        SquadNode->GetRebootCards().AddUnique(Card);
}

void UFortReplicationGraph::OnRebootCardDestroyed(AFortPlayerStateAthena* PS, ABuildingGameplayActorSpawnChip* Card)
{
    if (!PS->GetNetConnection())
        return;

    if (auto SquadNode = GetNodeForSquad(Card->GetSquadId()))
        SquadNode->GetRebootCards().RemoveSwap(Card);
}

void UFortReplicationGraph::OnPawnWeaponChange(AFortPawn* Pawn, AFortWeapon* NewWeapon, AFortWeapon* OldWeapon)
{
    if (OldWeapon)
        GetGlobalActorReplicationInfoMap().RemoveDependentActor(Pawn, OldWeapon);

    if (NewWeapon && IsActorValidForReplication(NewWeapon))
        GetGlobalActorReplicationInfoMap().AddDependentActor(Pawn, NewWeapon);
}

void UFortReplicationGraph::OnPetStateChange(AFortPlayerPawn* Pawn, AFortPlayerPetRepState* NewPet, AFortPlayerPetRepState* OldPet)
{
    if (NewPet)
        GetGlobalActorReplicationInfoMap().AddDependentActor(Pawn, (AActor*)NewPet);

    if (OldPet)
        GetGlobalActorReplicationInfoMap().RemoveDependentActor(Pawn, (AActor*)OldPet);
}

void UFortReplicationGraph::OnBalloonStateChange(AFortPlayerPawn* Pawn, ABuildingGameplayActor* NewBalloon, ABuildingGameplayActor* OldBalloon)
{
    if (NewBalloon)
        GetGlobalActorReplicationInfoMap().AddDependentActor(Pawn, (AActor*)NewBalloon);

    if (OldBalloon)
        GetGlobalActorReplicationInfoMap().RemoveDependentActor(Pawn, (AActor*)OldBalloon);
}

void UFortReplicationGraph::OnPossessedPropStateChange(AFortPlayerPawn* Pawn, AActor* NewPossessedProp, AActor* OldPossessedProp)
{
    auto& GlobalActorReplicationInfoMap = GetGlobalActorReplicationInfoMap();

    if (OldPossessedProp)
    {
        GlobalActorReplicationInfoMap.RemoveDependentActor(Pawn, OldPossessedProp);

        TArray<AActor*> AttachedActors;
        OldPossessedProp->GetAttachedActors(AttachedActors);

        for (auto AttachedActor : AttachedActors)
            GlobalActorReplicationInfoMap.RemoveDependentActor(Pawn, AttachedActor);
    }

    if (NewPossessedProp)
    {
        GlobalActorReplicationInfoMap.AddDependentActor(Pawn, NewPossessedProp);

        TArray<AActor*> AttachedActors;
        NewPossessedProp->GetAttachedActors(AttachedActors);

        for (auto AttachedActor : AttachedActors)
            GlobalActorReplicationInfoMap.AddDependentActor(Pawn, AttachedActor);
    }
}

void UFortReplicationGraph::OnCullDistSqChange(AActor* Actor, float DistSq)
{
    if (!Actor || !IsActorValidForReplicationGather(Actor))
        return;

    if (GetMappingPolicy(Actor->GetClass(), Actor) < EClassRepNodeMapping::Spatialize_Static)
        return;

    auto& GlobalActorReplicationInfoMap = GetGlobalActorReplicationInfoMap();
    const bool bWasCreated = !GlobalActorReplicationInfoMap.Find(Actor);
    auto& GlobalInfo = GlobalActorReplicationInfoMap.Get(Actor);

    if (FMath::Abs(GlobalInfo.GetCullDistanceSquared() - DistSq) <= 1.e-8f)
        return;

    const float OldDist = GlobalInfo.GetCullDistance();
    auto GridNode = GetRootGridNode();

    GlobalInfo.SetCullDistanceSquared(DistSq);

    if (bWasCreated)
        return;

    for (auto ConnectionManager : GetConnections())
    {
        if (auto ActorInfo = ConnectionManager->GetActorInfoMap().Find(Actor))
            ActorInfo->SetCullDistanceSquared(DistSq);
    }

    GridNode->NotifyActorCullDistChange(Actor, GlobalInfo, OldDist);
}

void UFortReplicationGraph::OnPawnCullDistSqChange(AFortPawn* Pawn, float DistSq)
{
    OnCullDistSqChange(Pawn, DistSq);
}

void UFortReplicationGraph::OnExitVehicleCullDistChange(AFortAthenaExitCraft* Craft, float DistSq)
{
    OnCullDistSqChange(Craft, DistSq);
}

void UFortReplicationGraph::OnPickupCullDistanceChanged(AFortPickup* Pickup, float Dist)
{
    OnCullDistSqChange(Pickup, Dist);
}

void UFortReplicationGraph::OnBuildingDistSqChange(ABuildingActor* Building, float Dist)
{
    OnCullDistSqChange(Building, (Dist + 1000.f) * (Dist + 1000.f));
}

void UFortReplicationGraph::OnActorNetCullDistanceChange(AActor* Actor)
{
    if (Actor)
        OnCullDistSqChange(Actor, Actor->GetNetCullDistanceSquared());
}

void UFortReplicationGraph::OnVehicleChanged(AFortPlayerPawn* Pawn, AActor* NewVehicle, AActor* PrevVehicle)
{
    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto& GlobalActorReplicationInfoMap = GetGlobalActorReplicationInfoMap();

    if (PrevVehicle)
    {
        GlobalActorReplicationInfoMap.RemoveDependentActor(Pawn, PrevVehicle);

        auto VehicleInterface = (IFortVehicleInterface*)PrevVehicle->GetInterfaceAddress(FortVehicleInterface);

        if (VehicleInterface && !VehicleInterface->ContainsPlayers() && PrevVehicle->GetOnDestroyed().bIsBound)
            PrevVehicle->GetOnDestroyed().bIsBound = FSparseDelegateStorage::Remove(PrevVehicle, FName(TEXT("OnDestroyed")), this, FName(TEXT("OnVehicleDestroyed")));
    }

    if (NewVehicle && IsActorValidForReplication(NewVehicle))
    {
        GlobalActorReplicationInfoMap.AddDependentActor(Pawn, NewVehicle);

        auto& PawnInfo = GlobalActorReplicationInfoMap.Get(Pawn);
        GlobalActorReplicationInfoMap.Get(NewVehicle);

        OnCullDistSqChange(NewVehicle, FMath::Max(NewVehicle->GetNetCullDistanceSquared(), PawnInfo.GetCullDistanceSquared()));

        FScriptDelegate OnVehicleDestroyedDelegate;
        OnVehicleDestroyedDelegate.BindUFunction(this, FName(TEXT("OnVehicleDestroyed")));

        NewVehicle->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::AddUnique(NewVehicle, FName(TEXT("OnDestroyed")), OnVehicleDestroyedDelegate);
    }
}

void UFortReplicationGraph::OnPlayerPawnHighFrequencyChange(AFortPlayerPawn* Pawn, bool bHighFreq)
{
    if (GetDisableHighFrequencyPawnRouting())
        return;

    FNewReplicatedActorInfo ActorInfo(Pawn);

    if (bHighFreq)
    {
        if (Pawn->GetTearOff())
            return;

        GetAlwaysRelevantNode()->NotifyAddNetworkActor(ActorInfo);

        FScriptDelegate OnHighFrequencyPawnDestroyedDelegate;
        OnHighFrequencyPawnDestroyedDelegate.BindUFunction(this, FName(TEXT("OnHighFrequencyPawnDestroyed")));

        Pawn->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::Add(Pawn, FName(TEXT("OnDestroyed")), OnHighFrequencyPawnDestroyedDelegate);
    }
    else
    {
        GetAlwaysRelevantNode()->NotifyRemoveNetworkActor(ActorInfo, true);

        if (Pawn->GetOnDestroyed().bIsBound)
            Pawn->GetOnDestroyed().bIsBound = FSparseDelegateStorage::RemoveAll(Pawn, FName(TEXT("OnDestroyed")), this);
    }
}

void UFortReplicationGraph::OnPlayerControllerStateChanged(AFortPlayerController* PC, const FName& OldState, const FName& NewState)
{
    static auto DynamicPlayerControllerFrequencies = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DynamicPlayerControllerFrequencies"));
    static auto PlayerControllerClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.PlayerController"));

    if (DynamicPlayerControllerFrequencies->GetInt() <= 0)
        return;

    auto& GlobalInfo = GetGlobalActorReplicationInfoMap().Get(PC);
    auto ControllerClass = NewState == FName(TEXT("Spectating")) ? PlayerControllerClass : PC->GetClass();
    auto DefaultController = (AActor*)ControllerClass->GetDefaultObject();
    const uint16 ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(DefaultController->GetNetUpdateFrequency());

    GlobalInfo.SetReplicationPeriodFrame(ReplicationPeriodFrame);

    for (auto ConnectionManager : GetConnections())
    {
        if (auto ActorInfo = ConnectionManager->GetActorInfoMap().Find(PC))
            ActorInfo->ReplicationPeriodFrame = ReplicationPeriodFrame;
    }
}

void UFortReplicationGraph::OnPhysicsObjectComponentActivated(UFortPhysicsObjectComponent* Component)
{
    static auto PhysicsObjectNetUpdateFrequencyOverride = IConsoleManager::Get().FindConsoleVariable(TEXT("FortPhysics.PhysicsObjectNetUpdateFrequencyOverride"));

    if (!Component)
        return;

    AActor* Owner = Component->GetOwner();

    if (!Owner || !IsActorValidForReplicationGather(Owner))
        return;

    const uint16 ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(PhysicsObjectNetUpdateFrequencyOverride->GetFloat());
    auto GlobalInfo = GetGlobalActorReplicationInfoMap().Find(Owner);

    if (!GlobalInfo)
        return;

    GlobalInfo->SetReplicationPeriodFrame(ReplicationPeriodFrame);

    for (auto ConnectionManager : GetConnections())
    {
        if (auto ActorInfo = ConnectionManager->GetActorInfoMap().Find(Owner))
            ActorInfo->ReplicationPeriodFrame = ReplicationPeriodFrame;
    }
}

void UFortReplicationGraph::OnPhysicsObjectComponentDeactivated(UFortPhysicsObjectComponent* Component)
{
    if (!Component)
        return;

    AActor* Owner = Component->GetOwner();

    if (!Owner || !IsActorValidForReplicationGather(Owner))
        return;

    const uint16 ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(Component->GetInactivateStateNetUpdateFrequency());
    auto GlobalInfo = GetGlobalActorReplicationInfoMap().Find(Owner);

    if (!GlobalInfo)
        return;

    GlobalInfo->SetReplicationPeriodFrame(ReplicationPeriodFrame);

    for (auto ConnectionManager : GetConnections())
    {
        if (auto ActorInfo = ConnectionManager->GetActorInfoMap().Find(Owner))
            ActorInfo->ReplicationPeriodFrame = ReplicationPeriodFrame;
    }
}

void UFortReplicationGraph::OnChangeOverrideActorDiscoveryBudget(IConsoleVariable* CVar)
{
    static auto OverrideActorDiscoveryBudget = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.OverrideActorDiscoveryBudget"));

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        auto ObjectItem = GUObjectArray.IndexToObject(Index);
        auto ReplicationGraph = ObjectItem && !ObjectItem->IsUnreachable() ? Cast<UFortReplicationGraph>((UObject*)ObjectItem->Object) : nullptr;

        if (ReplicationGraph && !(ReplicationGraph->GetFlags() & RF_ClassDefaultObject))
            ReplicationGraph->SetActorDiscoveryBudget(OverrideActorDiscoveryBudget->GetInt());
    }
}

void UFortReplicationGraph::OnNotifyAddReplicatedNonPlayerInventory(AActor* ParentActor, AFortInventory* Inventory)
{
    if (ParentActor && Inventory)
        GetGlobalActorReplicationInfoMap().AddDependentActor(ParentActor, Inventory);
}

void UFortReplicationGraph::OnNotifyRemoveReplicatedNonPlayerInventory(AActor* ParentActor, AFortInventory* Inventory)
{
    if (ParentActor && Inventory)
        GetGlobalActorReplicationInfoMap().RemoveDependentActor(ParentActor, Inventory);
}

void UFortReplicationGraph::SetAllSquadNodesEnabled(bool bEnabled)
{
    for (const auto& Pair : GetSquadMap())
    {
        if (auto SquadNode = Pair.Value)
            SquadNode->SetSpecialRelevancyEnabled(bEnabled);
    }
}

void UFortReplicationGraph::OnSpecialRelevancyEnabled(bool bEnabled)
{
    static auto HighFrequencyPawnUpdates = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.HighFrequencyPawnUpdates"));

    if (GetSpecialRelevancyEnabled() == bEnabled)
        return;

    GetSpecialRelevancyEnabled() = bEnabled;
    SetAllSquadNodesEnabled(GetSpecialRelevancyEnabled());

    if (HighFrequencyPawnUpdates->GetInt() <= 0)
        return;

    if (GetSpecialRelevancyEnabled())
        AFortPlayerPawn::GetNotifyPawnHighFrequencyUpdateChange().RemoveAll(this);
    else
        AFortPlayerPawn::GetNotifyPawnHighFrequencyUpdateChange().AddUObject(this, &UFortReplicationGraph::OnPlayerPawnHighFrequencyChange);
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

    static auto CellSize = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.CellSize"));
    static auto SpatialBiasX = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.SpatialBiasX"));
    static auto SpatialBiasY = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.SpatialBiasY"));
    static auto DisableSpatialRebuilds = IConsoleManager::Get().FindConsoleVariable(TEXT("FortRepGraph.DisableSpatialrebuilds"));

    GameState->GetOnCurrentPlaylistLoaded().RemoveAll(this);
    GetClassToNodetoRouteTo().Reset();
    GetDisableSquadNodes() = false;
    GetDisableHighFrequencyPawnRouting() = false;

    bool bSpecialRelevancyNode = false;
    auto SpecialRelevancyMutator = AFortAthenaMutator_SpecialRelevancy::StaticClass() ? Cast<AFortAthenaMutator_SpecialRelevancy>(GameState->GetMutatorByClass_Implementation(nullptr, AFortAthenaMutator_SpecialRelevancy::StaticClass())) : nullptr;

    if (SpecialRelevancyMutator)
    {
        if (auto RootGridNode = GetRootGridNode())
        {
            RemoveGlobalGraphNode(RootGridNode);
            RootGridNode->TearDown();
            GetRootGridNode() = nullptr;
        }

        UClass* NodeClass = SpecialRelevancyMutator->GetRootGridNodeClass();

        if (NodeClass && !NodeClass->IsChildOf(UFortReplicationGraphNode_SpecialRelevancy::StaticClass()))
            NodeClass = nullptr;

        auto SpecialRelevancyNode = (UFortReplicationGraphNode_SpecialRelevancy*)NewObject<UReplicationGraphNode>(this, NodeClass);
        SpecialRelevancyNode->GetOnSpecialRelevancyEnabled().AddUObject(this, &UFortReplicationGraph::OnSpecialRelevancyEnabled);
        GetRootGridNode() = SpecialRelevancyNode;
        InitNode(SpecialRelevancyNode);

        SpecialRelevancyNode->GetCellSize() = CellSize->GetFloat();
        TLWCGameLayout<float, double>::Store(SpecialRelevancyNode->GetSpatialBias(), SpatialBiasX->GetFloat());
        TLWCGameLayout<float, double>::Store(SpecialRelevancyNode->GetSpatialBias() + TLWCGameLayout<float, double>::Size(), SpatialBiasY->GetFloat());

        if (DisableSpatialRebuilds->GetInt())
            SpecialRelevancyNode->AddToClassRebuildDenyList(AActor::StaticClass());

        AddGlobalGraphNode(SpecialRelevancyNode);
        bSpecialRelevancyNode = true;
    }

    auto& GlobalRepGraphNodeClassesToAdd = GameMode->GetGlobalRepGraphNodeClassesToAdd();

    if (GlobalRepGraphNodeClassesToAdd.Num() <= 0)
    {
        if (bSpecialRelevancyNode)
            InitializeForWorld(this, World);

        return;
    }

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

UClass* UFortReplicationGraphNode_AlwaysRelevantForSquad::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_AlwaysRelevantForSquad"));

    return Class;
}

uint8& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetSquadID()
{
    return *((uint8*)this + GetSpecialRelevancyEnabledOffset() - 1);
}

TArray<AFortPlayerStateAthena*>& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetPlayerStates()
{
    return *(TArray<AFortPlayerStateAthena*>*)((uint8*)&GetRebootCards() - sizeof(TArray<AFortPlayerStateAthena*>));
}

TArray<ABuildingGameplayActorSpawnChip*>& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetRebootCards()
{
    return *(TArray<ABuildingGameplayActorSpawnChip*>*)((uint8*)&GetContainers() - sizeof(TArray<ABuildingGameplayActorSpawnChip*>));
}

TArray<ABuildingContainer*>& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetContainers()
{
    return *(TArray<ABuildingContainer*>*)((uint8*)&GetReplicationActorList() - sizeof(TArray<ABuildingContainer*>));
}

FActorRepListRefView& UFortReplicationGraphNode_AlwaysRelevantForSquad::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantForSquad::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"))
        .Find("lea *, [rcx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
}

bool UFortReplicationGraphNode_AlwaysRelevantForSquad::GetSpecialRelevancyEnabled()
{
    return (*((uint8*)this + GetSpecialRelevancyEnabledOffset()) & GetSpecialRelevancyEnabledMask()) != 0;
}

void UFortReplicationGraphNode_AlwaysRelevantForSquad::SetSpecialRelevancyEnabled(bool bEnabled)
{
    auto& Flags = *((uint8*)this + GetSpecialRelevancyEnabledOffset());

    Flags = (Flags & ~GetSpecialRelevancyEnabledMask()) | (bEnabled ? GetSpecialRelevancyEnabledMask() : 0);
}

int32 UFortReplicationGraphNode_AlwaysRelevantForSquad::GetSpecialRelevancyEnabledOffset()
{
    static auto SpecialRelevancyEnabled = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantForSquad::bSpecialRelevancyEnabled"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"))
        .FirstOf("cmp byte ptr [rbx+0x*], 0x0", "test byte ptr [rbx+0x*], 0x*")
        .Disp();

    return SpecialRelevancyEnabled;
}

uint8 UFortReplicationGraphNode_AlwaysRelevantForSquad::GetSpecialRelevancyEnabledMask()
{
    static auto Mask = (uint8)Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantForSquad::bSpecialRelevancyEnabled"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x10)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"))
        .FirstOf("cmp byte ptr [rbx+0x*], 0x0", "test byte ptr [rbx+0x*], 0x*")
        .Value();

    return Mask ? Mask : 1;
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

void UFortReplicationGraphNode_AlwaysRelevant_ForConnection::FlushStreamingAlwaysRelevantList(const FNewReplicatedActorInfo& ActorInfo)
{
    OnClientLevelVisibilityAdd(ActorInfo.StreamingLevelName, nullptr);
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

    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::AI, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::PhysicsObject, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
    DynamicTypeToNodeMap.Add(EFortDynamicNodeType::Vehicle, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>());
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

    if (GetCurrentDynamicNodeType() == EFortDynamicNodeType::AI)
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

UClass* UFortReplicationGraphNode_AlwaysRelevantForTeam::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_AlwaysRelevantForTeam"));

    return Class;
}

AFortTeamPrivateInfo*& UFortReplicationGraphNode_AlwaysRelevantForTeam::GetFortTeamPrivateInfo()
{
    return *(AFortTeamPrivateInfo**)((uint8*)&GetReplicationActorList() - sizeof(AFortTeamPrivateInfo*));
}

FActorRepListRefView& UFortReplicationGraphNode_AlwaysRelevantForTeam::GetReplicationActorList()
{
    static auto ReplicationActorList = Anchor::Offset(TEXT("UFortReplicationGraphNode_AlwaysRelevantForTeam::ReplicationActorList"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Entry(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForTeam"))
        .Find("lea rdx, [rcx+0x*]")
        .Disp();

    return *(FActorRepListRefView*)((uint8*)this + ReplicationActorList);
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
