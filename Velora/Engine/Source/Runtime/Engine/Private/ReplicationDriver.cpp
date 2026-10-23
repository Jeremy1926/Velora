#include "pch.h"
#include "Engine/ReplicationDriver.h"
#include "Engine/EngineBaseTypes.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"
#include "Detour.h"

UReplicationDriver* UReplicationDriver::CreateReplicationDriver(UNetDriver* NetDriver, const FURL& URL, UWorld* World)
{
    DETOUR(UReplicationDriver, CreateReplicationDriver, Anchor::Hook()
        .If(FEngineVersion::Current().GetChangelist() >= 19458861)
        .String(TEXT("ReplicationDriverClass is null! Not using ReplicationDriver."))
        .Optional()
        .Start()
        .Inline());

    BindCreateReplicationDriverDelegate();

    return DetourAs<DetourType>(DETOUR_ORIGINAL(UReplicationDriver, CreateReplicationDriver))(NetDriver, URL, World);
}

FCreateReplicationDriver& UReplicationDriver::CreateReplicationDriverDelegate()
{
    static auto CreateReplicationDriverDelegate = Anchor::Function<FCreateReplicationDriver& (*)()>(TEXT("UReplicationDriver::CreateReplicationDriverDelegate"))
        .String(TEXT("ReplicationDriverClass is null! Not using ReplicationDriver."))
        .Start()
        .Find("call 0x*")
        .Target()
        .Get();

    return CreateReplicationDriverDelegate();
}

void UReplicationDriver::FlushNetDormancy(AActor* Actor, bool bWasDormInitial)
{
    static auto FlushNetDormancy = Anchor::VirtualFunction<void (*)(UReplicationDriver*, AActor*, bool)>(TEXT("UReplicationDriver::FlushNetDormancy"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(this)
        .Get();

    FlushNetDormancy(this, Actor, bWasDormInitial);
}

int32 UReplicationDriver::ServerReplicateActors(float DeltaSeconds)
{
    static auto ServerReplicateActors = Anchor::VirtualFunction<int32 (*)(UReplicationDriver*, float)>(TEXT("UReplicationDriver::ProcessRemoteFunction"))
        .String(TEXT("UReplicationGraph::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed*"))
        .Start()
        .SlotIn(this)
        .LogSlot(TEXT("UReplicationDriver::ServerReplicateActors"), sizeof(void*))
        .Get(sizeof(void*));

    return ServerReplicateActors(this, DeltaSeconds);
}
