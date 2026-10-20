#include "pch.h"
#include "Engine/ReplicationDriver.h"
#include "Scanner.h"

void UReplicationDriver::FlushNetDormancy(AActor* Actor, bool bWasDormInitial)
{
    static auto FlushNetDormancy = Anchor::Slot(TEXT("UReplicationDriver::FlushNetDormancy"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(this)
        .Get();

    Scanner::Virtual<void (*)(UReplicationDriver*, AActor*, bool)>(this, FlushNetDormancy)(this, Actor, bWasDormInitial);
}

int32 UReplicationDriver::ServerReplicateActors(float DeltaSeconds)
{
    static auto ServerReplicateActors = [this]
    {
        auto ProcessRemoteFunction = Anchor::Slot(TEXT("UReplicationDriver::ProcessRemoteFunction"))
            .String(TEXT("UReplicationGraph::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed*"))
            .Start()
            .SlotIn(this)
            .Get();

        return Scanner::Slot(TEXT("UReplicationDriver::ServerReplicateActors"), ProcessRemoteFunction ? ProcessRemoteFunction + (int32)sizeof(void*) : 0);
    }();

    return Scanner::Virtual<int32 (*)(UReplicationDriver*, float)>(this, ServerReplicateActors)(this, DeltaSeconds);
}
