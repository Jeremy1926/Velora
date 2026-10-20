#include "pch.h"
#include "Engine/ReplicationDriver.h"
#include "Scanner.h"

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
