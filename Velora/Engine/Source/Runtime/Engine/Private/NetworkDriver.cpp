#include "pch.h"
#include "Engine/NetDriver.h"
#include "Engine/ActorChannel.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/NetConnection.h"
#include "Engine/NetworkObjectList.h"
#include "Engine/PackageMapClient.h"
#include "Engine/ReplicationDriver.h"
#include "Engine/World.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/EngineVersion.h"
#include "Misc/Parse.h"
#include "Templates/Sorting.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto TickFlushFunction = Scanner::String(TEXT("UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s")).Last("call 0x*").Target(TEXT("UNetDriver::TickFlush"));

static auto IsServerSlot = Scanner::At(TickFlushFunction).Find("call [rax+0x*]").Slot(TEXT("UNetDriver::IsServer"));

static auto TickFlush = Detour(TEXT("UNetDriver::TickFlush"), TickFlushFunction, &UNetDriver::TickFlush);

int32 UNetDriver::GetMaxInternetClientRate() const
{
    static auto MaxInternetClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxInternetClientRate")));

    return *MaxInternetClientRate->ContainerPtrToValuePtr<int32>(this);
}

int32 UNetDriver::GetMaxClientRate() const
{
    static auto MaxClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxClientRate")));

    return *MaxClientRate->ContainerPtrToValuePtr<int32>(this);
}

void UNetDriver::SetMaxClientRate(int32 InMaxClientRate)
{
    static auto MaxClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxClientRate")));

    *MaxClientRate->ContainerPtrToValuePtr<int32>(this) = InMaxClientRate;
}

float UNetDriver::GetServerTravelPause() const
{
    static auto ServerTravelPause = GetClass()->FindPropertyByName(FName(TEXT("ServerTravelPause")));

    return *ServerTravelPause->ContainerPtrToValuePtr<float>(this);
}

TArray<UNetConnection*>& UNetDriver::GetClientConnections()
{
    static auto ClientConnections = GetClass()->FindPropertyByName(FName(TEXT("ClientConnections")));

    return *ClientConnections->ContainerPtrToValuePtr<TArray<UNetConnection*>>(this);
}

UReplicationDriver* UNetDriver::GetReplicationDriver() const
{
    static auto ReplicationDriver = GetClass()->FindPropertyByName(FName(TEXT("ReplicationDriver")));

    return ReplicationDriver ? *ReplicationDriver->ContainerPtrToValuePtr<UReplicationDriver*>(this) : nullptr;
}

FName UNetDriver::GetNetDriverName() const
{
    static auto NetDriverName = GetClass()->FindPropertyByName(FName(TEXT("NetDriverName")));

    return *NetDriverName->ContainerPtrToValuePtr<FName>(this);
}

float UNetDriver::GetTime() const
{
    static auto Time = GetClass()->FindPropertyByName(FName(TEXT("Time")));

    return *Time->ContainerPtrToValuePtr<float>(this);
}

float UNetDriver::GetSpawnPrioritySeconds() const
{
    static auto SpawnPrioritySeconds = GetClass()->FindPropertyByName(FName(TEXT("SpawnPrioritySeconds")));

    return *SpawnPrioritySeconds->ContainerPtrToValuePtr<float>(this);
}

float UNetDriver::GetRelevantTimeout() const
{
    static auto RelevantTimeout = GetClass()->FindPropertyByName(FName(TEXT("RelevantTimeout")));

    return *RelevantTimeout->ContainerPtrToValuePtr<float>(this);
}

TSharedPtr<FNetGUIDCache>& UNetDriver::GetGuidCache()
{
    static auto GuidCache = Scanner::String(TEXT("ReadContentBlockHeader: Stably named sub-object not found.*")).Last("mov rcx, [rcx+0x*]").Disp(TEXT("UNetDriver::GuidCache"));

    return *(TSharedPtr<FNetGUIDCache>*)((uint8_t*)this + GuidCache);
}

static int32_t NetTagOffset()
{
    static auto NetTag = Scanner::Pattern("C7 ? ? 10 27 00 00 C7 ? ? 98 3A 00 00").Find("mov [*], e*; mov [*], *l").Disp(TEXT("UNetDriver::NetTag"));

    return NetTag;
}

int32 UNetDriver::GetNetTag() const
{
    return *(int32*)((uint8_t*)this + NetTagOffset());
}

void UNetDriver::SetNetTag(int32 InNetTag)
{
    *(int32*)((uint8_t*)this + NetTagOffset()) = InNetTag;
}

int32 FActorDestructionInfo::GetLevelSize()
{
    auto& Version = FEngineVersion::Current();

    return Version.GetMajor() > 4 || Version.GetMinor() >= 19 ? (int32)sizeof(TWeakObjectPtr<UObject>) : 0;
}

FActorDestructionInfo::FFields& FActorDestructionInfo::GetFields()
{
    return *(FFields*)((uint8*)this + GetLevelSize());
}

int32 FActorDestructionInfo::GetTypeSize()
{
    return GetLevelSize() + (int32)sizeof(FFields);
}

TWeakObjectPtr<UObject>& FActorDestructionInfo::GetObjOuter()
{
    return GetFields().ObjOuter;
}

FVector& FActorDestructionInfo::GetDestroyedPosition()
{
    return GetFields().DestroyedPosition;
}

FNetworkGUID& FActorDestructionInfo::GetNetGUID()
{
    return GetFields().NetGUID;
}

FString& FActorDestructionInfo::GetPathName()
{
    return GetFields().PathName;
}

FName& FActorDestructionInfo::GetStreamingLevelName()
{
    return GetFields().StreamingLevelName;
}

TMap<FNetworkGUID, FActorDestructionInfo>& UNetDriver::GetDestroyedStartupOrDormantActors()
{
    static auto DestroyedStartupOrDormantActors = []
    {
        auto AddClientConnection = Scanner::String(TEXT("AddClientConnection: Added client connection: %s"));
        auto Map = AddClientConnection.Find("lea r*, [*+0x???]");

        return Map.Get() ? Map.Disp(TEXT("UNetDriver::DestroyedStartupOrDormantActors")) : (int32)AddClientConnection.Find("add r*, 0x???").Imm(TEXT("UNetDriver::DestroyedStartupOrDormantActors"));
    }();

    return *(TMap<FNetworkGUID, FActorDestructionInfo>*)((uint8_t*)this + DestroyedStartupOrDormantActors);
}

static int32_t ReplicationFrameOffset()
{
    static auto ReplicationFrame = []
    {
        auto Frames = Scanner::String(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms.")).Start().Find("inc [*]; inc [*]");

        if (!Frames.Get())
            Frames = Scanner::String(TEXT("bPrioritizeActors and HasLevelStreamingFixes are both enabled*")).Start().Find("inc [*]; inc [*]");

        if (!Frames.Get())
            Frames = Scanner::String(TEXT("STAT_ReplayPrioritizeTime")).Last("inc [*]; inc [*]");

        return Frames.Find("inc [*]").Disp(TEXT("UNetDriver::ReplicationFrame"));
    }();

    return ReplicationFrame;
}

uint32 UNetDriver::GetReplicationFrame() const
{
    return *(uint32*)((uint8_t*)this + ReplicationFrameOffset());
}

void UNetDriver::SetReplicationFrame(uint32 InReplicationFrame)
{
    *(uint32*)((uint8_t*)this + ReplicationFrameOffset()) = InReplicationFrame;
}

FNetworkObjectList& UNetDriver::GetNetworkObjectList()
{
    static auto NetworkObjects = Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("xor edx, edx; mov rcx, *").Last("call 0x*").Call().Find("mov rcx, [*]; call 0x*").Disp(TEXT("UNetDriver::NetworkObjects"));

    return **(TSharedPtr<FNetworkObjectList>*)((uint8_t*)this + NetworkObjects);
}

bool UNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
    static auto InitListen = Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("call [*]").Slot(TEXT("UNetDriver::InitListen"));

    return Scanner::Virtual<bool (*)(UNetDriver*, FNetworkNotify*, FURL&, bool, FString&)>(this, InitListen)(this, InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void UNetDriver::SetWorld(UWorld* InWorld)
{
    static auto SetWorld = Scanner::Function<void (*)(UNetDriver*, UWorld*)>(Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("xor edx, edx; mov rcx, *").Last("call 0x*").Target(TEXT("UNetDriver::SetWorld")));

    SetWorld(this, InWorld);
}

void UNetDriver::TickFlush(float DeltaSeconds)
{
    static auto FlushActorDormancy = []
    {
        for (auto ActorFunction : { Scanner::At(Scanner::Exec(TEXT("/Script/Engine.Actor:FlushNetDormancy")).Find("jmp 0x*").Target(TEXT("AActor::FlushNetDormancy"))), Scanner::Native(TEXT("/Script/Engine.Actor:SetNetDormancy")) })
        {
            auto DemoNetDriverCall = ActorFunction.Find("ret").Last("call 0x*");

            while (DemoNetDriverCall.Get() && DemoNetDriverCall.Last("call 0x*").Target() != DemoNetDriverCall.Target())
                DemoNetDriverCall = DemoNetDriverCall.Last("call 0x*");

            auto GameFlushActorDormancy = Scanner::At(DemoNetDriverCall.Target());

            if (!GameFlushActorDormancy.Is("ret*") && !GameFlushActorDormancy.Is("jmp 0x*"))
                continue;

            DetourCall(TEXT("UNetDriver::FlushActorDormancy"), DemoNetDriverCall.Last("call 0x*").Get(), &UNetDriver::FlushActorDormancy);
            DetourCall(TEXT("UNetDriver::FlushActorDormancy"), DemoNetDriverCall.Get(), &UNetDriver::FlushActorDormancy);
        }

        return true;
    }();

    if (IsServer() && GetClientConnections().Num() > 0 && GetClientConnections()[0]->GetInternalAck() == false)
    {
        ServerReplicateActors(DeltaSeconds);
    }

    (this->*::TickFlush)(DeltaSeconds);
}

void UNetDriver::FlushActorDormancy(AActor* Actor, bool bWasDormInitial)
{
    static auto CVarSetNetDormancyEnabled = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.DormancyEnable"));

    if (CVarSetNetDormancyEnabled && CVarSetNetDormancyEnabled->GetValueOnAnyThread() == 0)
        return;

    if (auto ReplicationDriver = GetReplicationDriver())
    {
        ReplicationDriver->FlushNetDormancy(Actor, bWasDormInitial);
    }
    else
    {
        for (int32 i = 0; i < GetClientConnections().Num(); ++i)
        {
            UNetConnection* NetConnection = GetClientConnections()[i];
            if (NetConnection != NULL)
            {
                NetConnection->FlushDormancy(Actor);
            }
        }
    }
}

bool UNetDriver::IsServer() const
{
    return Scanner::Virtual<bool (*)(const UNetDriver*)>(this, IsServerSlot)(this);
}

ENetMode UNetDriver::GetNetMode() const
{
    return (IsServer() ? (GIsClient ? NM_ListenServer : NM_DedicatedServer) : NM_Client);
}

bool UNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
    static auto IsLevelInitializedForActor = Scanner::String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")).Start().Find("call [*]; test al, al").Slot(TEXT("UNetDriver::IsLevelInitializedForActor"));

    return Scanner::Virtual<bool (*)(const UNetDriver*, const AActor*, const UNetConnection*)>(this, IsLevelInitializedForActor)(this, InActor, InConnection);
}

bool UNetDriver::IsAdaptiveNetUpdateFrequencyEnabled()
{
    static auto CVarUseAdaptiveNetUpdateFrequency = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.UseAdaptiveNetUpdateFrequency"));

    const bool bUseAdapativeNetFrequency = CVarUseAdaptiveNetUpdateFrequency->GetValueOnAnyThread() > 0;

    return bUseAdapativeNetFrequency;
}

FNetViewer::FNetViewer(UNetConnection* InConnection, float DeltaSeconds)
    : Connection(InConnection)
    , InViewer(InConnection->GetPlayerController() ? (AActor*)InConnection->GetPlayerController() : InConnection->GetOwningActor())
    , ViewTarget(InConnection->GetViewTarget())
    , ViewLocation(ForceInit)
    , ViewDir(ForceInit)
{
    APlayerController* ViewingController = InConnection->GetPlayerController();

    ViewLocation = ViewTarget->GetActorLocation();

    if (ViewingController)
    {
        FRotator ViewRotation = ViewingController->GetControlRotation();
        ViewingController->GetPlayerViewPoint(ViewLocation, ViewRotation);
        ViewDir = ViewRotation.Vector();
    }
}

FActorPriority::FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth)
    : ActorInfo(InActorInfo)
    , Channel(InChannel)
    , DestructionInfo(nullptr)
{
    float Time = Channel ? (InConnection->GetDriver()->GetTime() - Channel->GetLastUpdateTime()) : InConnection->GetDriver()->GetSpawnPrioritySeconds();

    Priority = 0;

    for (int32 i = 0; i < Viewers.Num(); i++)
    {
        Priority = FMath::Max<int32>(Priority, FMath::RoundToInt(65536.0f * ActorInfo->Actor->GetNetPriority(Viewers[i].ViewLocation, Viewers[i].ViewDir, Viewers[i].InViewer, Viewers[i].ViewTarget, InChannel, Time, bLowBandwidth)));
    }
}

FActorPriority::FActorPriority(UNetConnection* InConnection, FActorDestructionInfo* Info, const TArray<FNetViewer>& Viewers)
    : ActorInfo(nullptr)
    , Channel(nullptr)
    , DestructionInfo(Info)
{
    Priority = 0;

    for (int32 i = 0; i < Viewers.Num(); i++)
    {
        float Time = InConnection->GetDriver()->GetSpawnPrioritySeconds();

        FVector Dir = DestructionInfo->GetDestroyedPosition() - Viewers[i].ViewLocation;
        float DistSq = Dir.SizeSquared();

        if ((Viewers[i].ViewDir | Dir) < 0.f)
        {
            if (DistSq > 2000.f * 2000.f)
                Time *= 0.2f;
            else if (DistSq > 500.f * 500.f)
                Time *= 0.4f;
        }
        else if (DistSq > 3162.f * 3162.f)
            Time *= 0.4f;

        Priority = FMath::Max<int32>(Priority, 65536.0f * Time);
    }
}

static FORCEINLINE bool IsActorRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers)
{
    for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++)
    {
        if (Actor->IsNetRelevantFor(ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation))
        {
            return true;
        }
    }

    return false;
}

static FORCEINLINE UNetConnection* IsActorOwnedByAndRelevantToConnection(const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget)
{
    const AActor* ActorOwner = Actor->GetNetOwner();

    bOutHasNullViewTarget = false;

    for (int i = 0; i < ConnectionViewers.Num(); i++)
    {
        UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

        if (ViewerConnection->GetViewTarget() == nullptr)
        {
            bOutHasNullViewTarget = true;
        }

        if (ActorOwner == ViewerConnection->GetPlayerController() ||
            (ViewerConnection->GetPlayerController() && ActorOwner == ViewerConnection->GetPlayerController()->GetPawn()) ||
            (ViewerConnection->GetViewTarget() && ViewerConnection->GetViewTarget()->IsRelevancyOwnerFor(Actor, ActorOwner, ViewerConnection->GetOwningActor())))
        {
            return ViewerConnection;
        }
    }

    return nullptr;
}

static FORCEINLINE bool IsActorDormant(FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection)
{
    return ActorInfo->GetDormantConnections().Contains(Connection);
}

static FORCEINLINE bool ShouldActorGoDormant(AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth)
{
    if (Actor->GetNetDormancy() <= DORM_Awake || !Channel || Channel->GetPendingDormancy() || Channel->GetDormant())
    {
        return false;
    }

    if (Actor->GetNetDormancy() == DORM_DormantPartial)
    {
        for (int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++)
        {
            if (!Actor->GetNetDormancy(ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth))
            {
                return false;
            }
        }
    }

    return true;
}

int32 UNetDriver::ServerReplicateActors_PrepConnections(const float DeltaSeconds)
{
    int32 NumClientsToTick = GetClientConnections().Num();

    static bool bForceClientTickingThrottle = FParse::Param(FCommandLine::Get(), TEXT("limitclientticks"));

    if (bForceClientTickingThrottle || GetNetMode() == NM_ListenServer)
    {
        static float DeltaTimeOverflow = 0.f;
        static bool LanPlay = FParse::Param(FCommandLine::Get(), TEXT("lanplay"));

        float ClientUpdatesThisFrame = GEngine->GetNetClientTicksPerSecond() * (DeltaSeconds + DeltaTimeOverflow) * (LanPlay ? 2.f : 1.f);
        NumClientsToTick = FMath::Min<int32>(NumClientsToTick, FMath::TruncToInt(ClientUpdatesThisFrame));

        if (NumClientsToTick == 0)
        {
            DeltaTimeOverflow += DeltaSeconds;
            return 0;
        }

        DeltaTimeOverflow = 0.f;
    }

    static auto MaxConnectionsToTickPerServerFrame = IConsoleManager::Get().FindConsoleVariable(TEXT("net.MaxConnectionsToTickPerServerFrame"));

    if (MaxConnectionsToTickPerServerFrame->GetInt() > 0)
    {
        NumClientsToTick = FMath::Min(GetClientConnections().Num(), MaxConnectionsToTickPerServerFrame->GetInt());
    }

    bool bFoundReadyConnection = false;

    for (int32 ConnIdx = 0; ConnIdx < GetClientConnections().Num(); ConnIdx++)
    {
        UNetConnection* Connection = GetClientConnections()[ConnIdx];

        AActor* OwningActor = Connection->GetOwningActor();

        if (OwningActor != nullptr && Connection->GetState() == USOCK_Open && (Connection->GetDriver()->GetTime() - Connection->GetLastReceiveTime() < 1.5f))
        {
            bFoundReadyConnection = true;

            AActor* DesiredViewTarget = OwningActor;

            if (Connection->GetPlayerController())
            {
                if (AActor* ViewTarget = Connection->GetPlayerController()->GetViewTarget())
                {
                    if (ViewTarget->GetWorld())
                    {
                        DesiredViewTarget = ViewTarget;
                    }
                }
            }

            Connection->SetViewTarget(DesiredViewTarget);

            for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ChildIdx++)
            {
                UNetConnection* Child = Connection->GetChildren()[ChildIdx];
                APlayerController* ChildPlayerController = Child->GetPlayerController();

                if (ChildPlayerController != nullptr)
                {
                    Child->SetViewTarget(ChildPlayerController->GetViewTarget());
                }
                else
                {
                    Child->SetViewTarget(nullptr);
                }
            }
        }
        else
        {
            Connection->SetViewTarget(nullptr);

            for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ChildIdx++)
            {
                Connection->GetChildren()[ChildIdx]->SetViewTarget(nullptr);
            }
        }
    }

    return bFoundReadyConnection ? NumClientsToTick : 0;
}

void UNetDriver::ServerReplicateActors_BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime)
{
    int32 NumInitiallyDormant = 0;

    const bool bUseAdapativeNetFrequency = IsAdaptiveNetUpdateFrequencyEnabled();

    TArray<AActor*> ActorsToRemove;

    for (const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList().GetActiveObjects())
    {
        FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();

        if (!ActorInfo->GetPendingNetUpdate() && GetWorld()->GetTimeSeconds() <= ActorInfo->GetNextUpdateTime())
        {
            continue;
        }

        AActor* Actor = ActorInfo->Actor;

        if (Actor->IsPendingKillPending())
        {
            ActorsToRemove.Add(Actor);
            continue;
        }

        if (Actor->GetRemoteRole() == ROLE_None)
        {
            ActorsToRemove.Add(Actor);
            continue;
        }

        if (Actor->GetNetDriverName() != GetNetDriverName())
        {
            continue;
        }

        if (!Actor->IsActorInitialized())
        {
            continue;
        }

        ULevel* Level = Actor->GetLevel();

        if (Level->HasVisibilityChangeRequestPending() || Level->IsAssociatingLevel())
        {
            continue;
        }

        if (Actor->GetNetDormancy() == DORM_Initial && Actor->IsNetStartupActor())
        {
            NumInitiallyDormant++;
            ActorsToRemove.Add(Actor);
            continue;
        }

        if (ActorInfo->GetLastNetReplicateTime() == 0)
        {
            ActorInfo->GetLastNetReplicateTime() = GetWorld()->GetTimeSeconds();
            ActorInfo->GetOptimalNetUpdateDelta() = 1.0f / Actor->GetNetUpdateFrequency();
        }

        const float LastReplicateDelta = GetWorld()->GetTimeSeconds() - ActorInfo->GetLastNetReplicateTime();

        if (LastReplicateDelta > 2.0f)
        {
            if (Actor->GetMinNetUpdateFrequency() == 0.0f)
            {
                Actor->SetMinNetUpdateFrequency(2.0f);
            }

            const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
            const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);

            const float Alpha = FMath::Clamp((LastReplicateDelta - 2.0f) / 5.0f, 0.0f, 1.0f);
            ActorInfo->GetOptimalNetUpdateDelta() = FMath::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
        }

        if (!ActorInfo->GetPendingNetUpdate())
        {
            const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->GetOptimalNetUpdateDelta() : 1.0f / Actor->GetNetUpdateFrequency();

            ActorInfo->GetNextUpdateTime() = GetWorld()->GetTimeSeconds() + FMath::SRand() * ServerTickTime + NextUpdateDelta;

            ActorInfo->GetLastNetUpdateTime() = GetTime();
        }

        ActorInfo->SetPendingNetUpdate(false);

        OutConsiderList.Add(ActorInfo);

        Actor->CallPreReplication(this);
    }

    for (AActor* Actor : ActorsToRemove)
    {
        GetNetworkObjectList().Remove(Actor);
    }
}

int32 UNetDriver::ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors)
{
    static auto CVarSetNetDormancyEnabled = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("net.DormancyEnable"));

    SetNetTag(GetNetTag() + 1);
    Connection->SetTickCount(Connection->GetTickCount() + 1);

    for (int32 j = 0; j < Connection->GetSentTemporaries().Num(); j++)
    {
        Connection->GetSentTemporaries()[j]->SetNetTag(GetNetTag());
    }

    int32 FinalSortedCount = 0;
    int32 DeletedCount = 0;

    TWeakObjectPtr<UNetConnection> WeakConnection(Connection);

    const int32 MaxSortedActors = ConsiderList.Num() + GetDestroyedStartupOrDormantActors().Num();

    if (MaxSortedActors > 0)
    {
        OutPriorityList = new FActorPriority[MaxSortedActors];
        OutPriorityActors = new FActorPriority*[MaxSortedActors];

        AGameNetworkManager* const NetworkManager = GetWorld()->GetNetworkManager();
        const bool bLowNetBandwidth = NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false;

        for (FNetworkObjectInfo* ActorInfo : ConsiderList)
        {
            AActor* Actor = ActorInfo->Actor;

            UActorChannel* Channel = Connection->GetActorChannels().FindRef(ActorInfo->GetWeakActor());

            if (!Channel)
            {
                if (!IsLevelInitializedForActor(Actor, Connection))
                {
                    continue;
                }

                if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
                {
                    continue;
                }
            }

            UNetConnection* PriorityConnection = Connection;

            if (Actor->GetOnlyRelevantToOwner())
            {
                bool bHasNullViewTarget = false;

                PriorityConnection = IsActorOwnedByAndRelevantToConnection(Actor, ConnectionViewers, bHasNullViewTarget);

                if (PriorityConnection == nullptr)
                {
                    if (!bHasNullViewTarget && Channel != nullptr && GetTime() - Channel->GetRelevantTime() >= GetRelevantTimeout())
                    {
                        Channel->Close();
                    }

                    continue;
                }
            }
            else if (!CVarSetNetDormancyEnabled || CVarSetNetDormancyEnabled->GetValueOnGameThread() != 0)
            {
                if (IsActorDormant(ActorInfo, WeakConnection))
                {
                    continue;
                }

                if (ShouldActorGoDormant(Actor, ConnectionViewers, Channel, GetTime(), bLowNetBandwidth))
                {
                    Channel->StartBecomingDormant();
                }
            }

            if (Actor->GetNetTag() != GetNetTag())
            {
                Actor->SetNetTag(GetNetTag());

                OutPriorityList[FinalSortedCount] = FActorPriority(PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth);
                OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

                FinalSortedCount++;
            }
        }

        for (const FNetworkGUID& NetGUID : Connection->GetDestroyedStartupOrDormantActors())
        {
            FActorDestructionInfo& DInfo = GetDestroyedStartupOrDormantActors().FindChecked(NetGUID);
            OutPriorityList[FinalSortedCount] = FActorPriority(Connection, &DInfo, ConnectionViewers);
            OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
            FinalSortedCount++;
            DeletedCount++;
        }

        Sort(OutPriorityActors, FinalSortedCount, FCompareFActorPriority());
    }

    return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated)
{
    int32 ActorUpdatesThisConnection = 0;
    int32 ActorUpdatesThisConnectionSent = 0;
    int32 FinalRelevantCount = 0;

    if (!Connection->IsNetReady(0))
    {
        return 0;
    }

    for (int32 j = 0; j < FinalSortedCount; j++)
    {
        FNetworkObjectInfo* ActorInfo = PriorityActors[j]->ActorInfo;

        if (ActorInfo == nullptr && PriorityActors[j]->DestructionInfo)
        {
            if (PriorityActors[j]->DestructionInfo->GetStreamingLevelName() != NAME_None && !Connection->GetClientVisibleLevelNames().Contains(PriorityActors[j]->DestructionInfo->GetStreamingLevelName()))
            {
                continue;
            }

            UActorChannel* Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);

            if (Channel)
            {
                FinalRelevantCount++;

                Channel->SetChannelActorForDestroy(PriorityActors[j]->DestructionInfo);
                Connection->GetDestroyedStartupOrDormantActors().Remove(PriorityActors[j]->DestructionInfo->GetNetGUID());
            }

            continue;
        }

        UActorChannel* Channel = PriorityActors[j]->Channel;

        if (!Channel || Channel->GetActor())
        {
            AActor* Actor = ActorInfo->Actor;
            bool bIsRelevant = false;

            const bool bLevelInitializedForActor = IsLevelInitializedForActor(Actor, Connection);

            if (bLevelInitializedForActor)
            {
                if (!Actor->GetTearOff() && (!Channel || GetTime() - Channel->GetRelevantTime() > 1.f))
                {
                    if (IsActorRelevantToConnection(Actor, ConnectionViewers))
                    {
                        bIsRelevant = true;
                    }
                }
            }

            const bool bIsRecentlyRelevant = bIsRelevant || (Channel && GetTime() - Channel->GetRelevantTime() < GetRelevantTimeout()) || ActorInfo->GetForceRelevantNextUpdate();

            ActorInfo->SetForceRelevantNextUpdate(false);

            if (bIsRecentlyRelevant)
            {
                FinalRelevantCount++;

                if (Channel == nullptr && GetGuidCache()->SupportsObject(Actor->GetClass()) && GetGuidCache()->SupportsObject(Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype()))
                {
                    if (bLevelInitializedForActor)
                    {
                        Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);

                        if (Channel)
                        {
                            Channel->SetChannelActor(Actor);
                        }
                    }
                    else if (Actor->GetNetUpdateFrequency() < 1.0f)
                    {
                        ActorInfo->GetNextUpdateTime() = Actor->GetWorld()->GetTimeSeconds() + 0.2f * FMath::FRand();
                    }
                }

                if (Channel)
                {
                    if (bIsRelevant)
                    {
                        Channel->SetRelevantTime(GetTime() + 0.5f * FMath::SRand());
                    }

                    if (Channel->IsNetReady(0))
                    {
                        if (Channel->ReplicateActor())
                        {
                            ActorUpdatesThisConnectionSent++;

                            const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
                            const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);
                            const float DeltaBetweenReplications = (GetWorld()->GetTimeSeconds() - ActorInfo->GetLastNetReplicateTime());

                            ActorInfo->GetOptimalNetUpdateDelta() = FMath::Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
                            ActorInfo->GetLastNetReplicateTime() = GetWorld()->GetTimeSeconds();
                        }

                        ActorUpdatesThisConnection++;
                        OutUpdated++;
                    }
                    else
                    {
                        Actor->ForceNetUpdate();
                    }

                    if (!Connection->IsNetReady(0))
                    {
                        return j;
                    }
                }
            }

            if ((!bIsRecentlyRelevant || Actor->GetTearOff()) && Channel != nullptr)
            {
                if (!bLevelInitializedForActor || !Actor->IsNetStartupActor())
                {
                    Channel->Close();
                }
            }
        }
    }

    return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors(float DeltaSeconds)
{
    if (GetClientConnections().Num() == 0)
    {
        return 0;
    }

    if (auto ReplicationDriver = GetReplicationDriver())
    {
        return ReplicationDriver->ServerReplicateActors(DeltaSeconds);
    }

    int32 Updated = 0;

    SetReplicationFrame(GetReplicationFrame() + 1);

    const int32 NumClientsToTick = ServerReplicateActors_PrepConnections(DeltaSeconds);

    if (NumClientsToTick == 0)
    {
        return 0;
    }

    AWorldSettings* WorldSettings = GetWorld()->GetWorldSettings();

    bool bCPUSaturated = false;
    float ServerTickTime = GEngine->GetMaxTickRate(DeltaSeconds);

    if (ServerTickTime == 0.f)
    {
        ServerTickTime = DeltaSeconds;
    }
    else
    {
        ServerTickTime = 1.f / ServerTickTime;
        bCPUSaturated = DeltaSeconds > 1.2f * ServerTickTime;
    }

    TArray<FNetworkObjectInfo*> ConsiderList;
    ConsiderList.Reserve(GetNetworkObjectList().GetActiveObjects().Num());

    ServerReplicateActors_BuildConsiderList(ConsiderList, ServerTickTime);

    for (int32 i = 0; i < GetClientConnections().Num(); i++)
    {
        UNetConnection* Connection = GetClientConnections()[i];

        if (i >= NumClientsToTick)
        {
            for (int32 ConsiderIdx = 0; ConsiderIdx < ConsiderList.Num(); ConsiderIdx++)
            {
                AActor* Actor = ConsiderList[ConsiderIdx]->Actor;

                if (Actor != nullptr && !ConsiderList[ConsiderIdx]->GetPendingNetUpdate())
                {
                    UActorChannel* Channel = Connection->GetActorChannels().FindRef(ConsiderList[ConsiderIdx]->GetWeakActor());

                    if (Channel != nullptr && Channel->GetLastUpdateTime() < ConsiderList[ConsiderIdx]->GetLastNetUpdateTime())
                    {
                        ConsiderList[ConsiderIdx]->SetPendingNetUpdate(true);
                    }
                }
            }

            Connection->SetTimeSensitive(false);
        }
        else if (Connection->GetViewTarget())
        {
            TArray<FNetViewer>& ConnectionViewers = WorldSettings->GetReplicationViewers();

            ConnectionViewers.Reset();
            new(ConnectionViewers) FNetViewer(Connection, DeltaSeconds);

            for (int32 ViewerIndex = 0; ViewerIndex < Connection->GetChildren().Num(); ViewerIndex++)
            {
                if (Connection->GetChildren()[ViewerIndex]->GetViewTarget() != nullptr)
                {
                    new(ConnectionViewers) FNetViewer(Connection->GetChildren()[ViewerIndex], DeltaSeconds);
                }
            }

            if (Connection->GetPlayerController())
            {
                Connection->GetPlayerController()->SendClientAdjustment();
            }

            for (int32 ChildIdx = 0; ChildIdx < Connection->GetChildren().Num(); ChildIdx++)
            {
                if (Connection->GetChildren()[ChildIdx]->GetPlayerController() != nullptr)
                {
                    Connection->GetChildren()[ChildIdx]->GetPlayerController()->SendClientAdjustment();
                }
            }

            FActorPriority* PriorityList = nullptr;
            FActorPriority** PriorityActors = nullptr;

            const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors(Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors);

            const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors(Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated);

            for (int32 k = LastProcessedActor; k < FinalSortedCount; k++)
            {
                if (!PriorityActors[k]->ActorInfo)
                {
                    continue;
                }

                AActor* Actor = PriorityActors[k]->ActorInfo->Actor;

                UActorChannel* Channel = PriorityActors[k]->Channel;

                if (Channel != nullptr && GetTime() - Channel->GetRelevantTime() <= 1.f)
                {
                    PriorityActors[k]->ActorInfo->SetPendingNetUpdate(true);
                }
                else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
                {
                    PriorityActors[k]->ActorInfo->SetPendingNetUpdate(true);

                    if (Channel != nullptr)
                    {
                        Channel->SetRelevantTime(GetTime() + 0.5f * FMath::SRand());
                    }
                }
            }

            delete[] PriorityList;
            delete[] PriorityActors;

            ConnectionViewers.Reset();
        }
    }

    if (NumClientsToTick < GetClientConnections().Num())
    {
        int32 NumConnectionsToMove = NumClientsToTick;

        while (NumConnectionsToMove > 0)
        {
            UNetConnection* Connection = GetClientConnections()[0];
            GetClientConnections().RemoveAt(0, 1);
            GetClientConnections().Add(Connection);
            NumConnectionsToMove--;
        }
    }

    return Updated;
}
