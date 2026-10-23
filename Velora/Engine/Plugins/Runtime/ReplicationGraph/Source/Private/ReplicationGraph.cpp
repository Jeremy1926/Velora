#include "pch.h"
#include "ReplicationGraph.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
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
