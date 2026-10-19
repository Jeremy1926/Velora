#include "pch.h"
#include "Engine/ReplicationDriver.h"
#include "Scanner.h"

void UReplicationDriver::FlushNetDormancy(AActor* Actor, bool bWasDormInitial)
{
    static auto FlushNetDormancy = [this]
    {
        auto Function = Scanner::String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*")).Start().Get();

        return Scanner::Slot(TEXT("UReplicationDriver::FlushNetDormancy"), *(const void***)this, [&](const Scanner::FCode& Entry) { return Entry.Get() == Function; });
    }();

    Scanner::Virtual<void (*)(UReplicationDriver*, AActor*, bool)>(this, FlushNetDormancy)(this, Actor, bWasDormInitial);
}

int32 UReplicationDriver::ServerReplicateActors(float DeltaSeconds)
{
    static auto ServerReplicateActors = [this]
    {
        auto Function = Scanner::String(TEXT("UReplicationGraph::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed*")).Start().Get();
        auto ProcessRemoteFunction = Scanner::Slot(TEXT("UReplicationDriver::ProcessRemoteFunction"), *(const void***)this, [&](const Scanner::FCode& Entry) { return Entry.Get() == Function; });

        return Scanner::Slot(TEXT("UReplicationDriver::ServerReplicateActors"), ProcessRemoteFunction ? ProcessRemoteFunction + (int32)sizeof(void*) : 0);
    }();

    return Scanner::Virtual<int32 (*)(UReplicationDriver*, float)>(this, ServerReplicateActors)(this, DeltaSeconds);
}
