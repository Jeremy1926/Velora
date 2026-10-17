#include "pch.h"
#include "Engine/NetDriver.h"
#include "Engine/ActorChannel.h"
#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/NetConnection.h"
#include "Engine/NetworkObjectList.h"
#include "Engine/PackageMapClient.h"
#include "Engine/World.h"
#include "GameFramework/GameNetworkManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Templates/Sorting.h"
#include "UObject/UnrealType.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto TickFlushFunction = []
{
    auto Reference = Scanner::FindStringReference(TEXT("UDemoNetDriver::TickFlush: ReplayStreamer ERROR: %s"));
    auto Function = Scanner::FindFunction(Reference);
    const uint8_t* Call = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            Call = Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }

    return Call;
}();

static auto IsServerSlot = []
{
    auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(TickFlushFunction));

    for (auto Address = TickFlushFunction; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
            return (int32_t)Instruction.disp.disp32;

        Address += Length;
    }

    return 0;
}();

static auto TickFlush = Detour(TickFlushFunction, &UNetDriver::TickFlush);

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
    static auto GuidCache = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("ReadContentBlockHeader: Stably named sub-object not found. Component: %s, Actor: %s"));
        auto Function = Scanner::FindFunction(Reference);
        int32_t Offset = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm == 0x89)
                Offset = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return Offset;
    }();

    return *(TSharedPtr<FNetGUIDCache>*)((uint8_t*)this + GuidCache);
}

static int32_t NetTagOffset()
{
    static auto NetTag = []
    {
        for (auto Address = Scanner::FindPattern("C7 43 ? 10 27 00 00 C7 43 ? 98 3A 00 00");;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x89 && Instruction.modrm_mod == 2)
            {
                hde64s Next;
                hde64_disasm(Address + Length, &Next);

                if (Next.opcode == 0x88 && Next.modrm_mod == 2 && Next.disp.disp32 == Instruction.disp.disp32 + sizeof(int32))
                    return (int32_t)Instruction.disp.disp32;
            }

            Address += Length;
        }
    }();

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

TMap<FNetworkGUID, FActorDestructionInfo>& UNetDriver::GetDestroyedStartupOrDormantActors()
{
    static auto DestroyedStartupOrDormantActors = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("AddClientConnection: Added client connection: %s"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x8D && Instruction.rex_w && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    return *(TMap<FNetworkGUID, FActorDestructionInfo>*)((uint8_t*)this + DestroyedStartupOrDormantActors);
}

static int32_t ReplicationFrameOffset()
{
    static auto ReplicationFrame = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms.")));
        auto End = Scanner::FindFunctionEnd(Function);
        bool bIncrement = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);
            auto bIncrementDword = Instruction.opcode == 0xFF && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 2;

            if (bIncrement && bIncrementDword)
                return (int32_t)Instruction.disp.disp32;

            bIncrement = bIncrementDword;
            Address += Length;
        }

        return 0;
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
    static auto NetworkObjects = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        const uint8_t* SetWorld = nullptr;
        const uint8_t* Call = nullptr;
        bool bDynamicSourceLevels = false;

        for (auto Address = Reference; Address < End && !SetWorld;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bDynamicSourceLevels)
                    SetWorld = Call;

                Call = Address + Length + (int32_t)Instruction.imm.imm32;
            }
            else if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
                bDynamicSourceLevels = true;

            Address += Length;
        }

        auto SetWorldEnd = Scanner::FindFunctionEnd(Scanner::FindFunction(SetWorld));
        int32_t Offset = 0;
        int32_t Load = 0;

        for (auto Address = SetWorld; Address < SetWorldEnd;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && Load)
                Offset = Load;

            Load = Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 2 ? (int32_t)Instruction.disp.disp32 : 0;
            Address += Length;
        }

        return Offset;
    }();

    return **(TSharedPtr<FNetworkObjectList>*)((uint8_t*)this + NetworkObjects);
}

bool UNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
    static auto InitListen = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(UNetDriver*, FNetworkNotify*, FURL&, bool, FString&))(VTable + InitListen))(this, InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void UNetDriver::SetWorld(UWorld* InWorld)
{
    static auto SetWorld = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        const uint8_t* Call = nullptr;
        bool bDynamicSourceLevels = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bDynamicSourceLevels)
                    return (void (*)(UNetDriver*, UWorld*))Call;

                Call = Address + Length + (int32_t)Instruction.imm.imm32;
            }
            else if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
                bDynamicSourceLevels = true;

            Address += Length;
        }

        return (void (*)(UNetDriver*, UWorld*))nullptr;
    }();

    SetWorld(this, InWorld);
}

void UNetDriver::TickFlush(float DeltaSeconds)
{
    if (IsServer() && GetClientConnections().Num() > 0 && GetClientConnections()[0]->GetInternalAck() == false)
    {
        ServerReplicateActors(DeltaSeconds);
    }

    (this->*::TickFlush)(DeltaSeconds);
}

bool UNetDriver::IsServer() const
{
    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UNetDriver*))(VTable + IsServerSlot))(this);
}

ENetMode UNetDriver::GetNetMode() const
{
    return (IsServer() ? (GIsClient ? NM_ListenServer : NM_DedicatedServer) : NM_Client);
}

bool UNetDriver::IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const
{
    static auto IsLevelInitializedForActor = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")));
        auto End = Scanner::FindFunctionEnd(Function);
        int32_t Slot = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Slot && Instruction.opcode == 0x84 && Instruction.modrm == 0xC0)
                return Slot;

            Slot = Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2 ? (int32_t)Instruction.disp.disp32 : 0;
            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UNetDriver*, const AActor*, const UNetConnection*))(VTable + IsLevelInitializedForActor))(this, InActor, InConnection);
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

        FVector Dir = DestructionInfo->DestroyedPosition - Viewers[i].ViewLocation;
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
    return ActorInfo->DormantConnections.Contains(Connection);
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

        if (!ActorInfo->bPendingNetUpdate && GetWorld()->GetTimeSeconds() <= ActorInfo->NextUpdateTime)
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

        if (ActorInfo->LastNetReplicateTime == 0)
        {
            ActorInfo->LastNetReplicateTime = GetWorld()->GetTimeSeconds();
            ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->GetNetUpdateFrequency();
        }

        const float LastReplicateDelta = GetWorld()->GetTimeSeconds() - ActorInfo->LastNetReplicateTime;

        if (LastReplicateDelta > 2.0f)
        {
            if (Actor->GetMinNetUpdateFrequency() == 0.0f)
            {
                Actor->SetMinNetUpdateFrequency(2.0f);
            }

            const float MinOptimalDelta = 1.0f / Actor->GetNetUpdateFrequency();
            const float MaxOptimalDelta = FMath::Max(1.0f / Actor->GetMinNetUpdateFrequency(), MinOptimalDelta);

            const float Alpha = FMath::Clamp((LastReplicateDelta - 2.0f) / 5.0f, 0.0f, 1.0f);
            ActorInfo->OptimalNetUpdateDelta = FMath::Lerp(MinOptimalDelta, MaxOptimalDelta, Alpha);
        }

        if (!ActorInfo->bPendingNetUpdate)
        {
            const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->GetNetUpdateFrequency();

            ActorInfo->NextUpdateTime = GetWorld()->GetTimeSeconds() + FMath::SRand() * ServerTickTime + NextUpdateDelta;

            ActorInfo->LastNetUpdateTime = GetTime();
        }

        ActorInfo->bPendingNetUpdate = false;

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

            UActorChannel* Channel = Connection->GetActorChannels().FindRef(ActorInfo->WeakActor);

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
            else if (CVarSetNetDormancyEnabled->GetValueOnGameThread() != 0)
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
            if (PriorityActors[j]->DestructionInfo->StreamingLevelName != NAME_None && !Connection->GetClientVisibleLevelNames().Contains(PriorityActors[j]->DestructionInfo->StreamingLevelName))
            {
                continue;
            }

            UActorChannel* Channel = (UActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);

            if (Channel)
            {
                FinalRelevantCount++;

                Channel->SetChannelActorForDestroy(PriorityActors[j]->DestructionInfo);
                Connection->GetDestroyedStartupOrDormantActors().Remove(PriorityActors[j]->DestructionInfo->NetGUID);
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

            const bool bIsRecentlyRelevant = bIsRelevant || (Channel && GetTime() - Channel->GetRelevantTime() < GetRelevantTimeout()) || ActorInfo->bForceRelevantNextUpdate;

            ActorInfo->bForceRelevantNextUpdate = false;

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
                        ActorInfo->NextUpdateTime = Actor->GetWorld()->GetTimeSeconds() + 0.2f * FMath::FRand();
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
                            const float DeltaBetweenReplications = (GetWorld()->GetTimeSeconds() - ActorInfo->LastNetReplicateTime);

                            ActorInfo->OptimalNetUpdateDelta = FMath::Clamp(DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta);
                            ActorInfo->LastNetReplicateTime = GetWorld()->GetTimeSeconds();
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

                if (Actor != nullptr && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
                {
                    UActorChannel* Channel = Connection->GetActorChannels().FindRef(ConsiderList[ConsiderIdx]->WeakActor);

                    if (Channel != nullptr && Channel->GetLastUpdateTime() < ConsiderList[ConsiderIdx]->LastNetUpdateTime)
                    {
                        ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
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
                    PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
                }
                else if (IsActorRelevantToConnection(Actor, ConnectionViewers))
                {
                    PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;

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
