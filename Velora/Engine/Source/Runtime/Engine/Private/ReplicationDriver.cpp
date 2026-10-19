#include "pch.h"
#include "Engine/ReplicationDriver.h"
#include "Scanner.h"

void UReplicationDriver::FlushNetDormancy(AActor* Actor, bool bWasDormInitial)
{
    static auto FlushNetDormancy = [this]
    {
        auto VTable = *(const uint8_t***)this;
        auto Function = Scanner::String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*")).Start().Get();
        int32 Index = 0;

        while (Index < 512 && VTable[Index] != Function)
            Index++;

        return Scanner::Slot(TEXT("UReplicationDriver::FlushNetDormancy"), Index < 512 ? Index * (int32)sizeof(void*) : 0);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(UReplicationDriver*, AActor*, bool))(VTable + FlushNetDormancy))(this, Actor, bWasDormInitial);
}

int32 UReplicationDriver::ServerReplicateActors(float DeltaSeconds)
{
    static auto ServerReplicateActors = [this]
    {
        auto VTable = *(const uint8_t***)this;
        auto ProcessRemoteFunction = Scanner::String(TEXT("UReplicationGraph::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed*")).Start().Get();
        int32 Index = 0;

        while (Index < 512 && VTable[Index] != ProcessRemoteFunction)
            Index++;

        return Scanner::Slot(TEXT("UReplicationDriver::ServerReplicateActors"), Index < 512 ? (Index + 1) * (int32)sizeof(void*) : 0);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(UReplicationDriver*, float))(VTable + ServerReplicateActors))(this, DeltaSeconds);
}
