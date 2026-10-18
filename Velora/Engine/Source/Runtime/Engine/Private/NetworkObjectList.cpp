#include "pch.h"
#include "Engine/NetworkObjectList.h"
#include "Scanner.h"

void FNetworkObjectList::Remove(AActor* const Actor)
{
    static auto Remove = (void (*)(FNetworkObjectList*, AActor*))Scanner::String(TEXT("NotifyActorDestroyed %s - StartupActor")).Find("mov rcx, [*]; call 0x*").Call().Get(TEXT("FNetworkObjectList::Remove"));

    Remove(this, Actor);
}

int32 FNetworkObjectInfo::GetFlagsOffset()
{
    static auto FlagsOffset = Scanner::String(TEXT("FNetworkObjectList::Add: Adding actor. Actor: %s, Total: %i, Active: %i, NetDriverName: %s")).Start().Find("mov ecx, 0x*").Call(2).Find("and dword ptr [*], 0xfffffffc").Disp(TEXT("FNetworkObjectInfo::bPendingNetUpdate"));

    return FlagsOffset;
}

TWeakObjectPtr<AActor> FNetworkObjectInfo::GetWeakActor() const
{
    if (GetFlagsOffset() > 0x20)
        return *(TWeakObjectPtr<AActor>*)((uint8*)this + sizeof(AActor*));

    return TWeakObjectPtr<AActor>(Actor);
}

double& FNetworkObjectInfo::GetNextUpdateTime()
{
    return *(double*)((uint8*)this + GetFlagsOffset() - 0x18);
}

double& FNetworkObjectInfo::GetLastNetReplicateTime()
{
    return *(double*)((uint8*)this + GetFlagsOffset() - 0x10);
}

float& FNetworkObjectInfo::GetOptimalNetUpdateDelta()
{
    return *(float*)((uint8*)this + GetFlagsOffset() - 0x8);
}

float& FNetworkObjectInfo::GetLastNetUpdateTime()
{
    return *(float*)((uint8*)this + GetFlagsOffset() - 0x4);
}

bool FNetworkObjectInfo::GetPendingNetUpdate() const
{
    return *(const uint32*)((const uint8*)this + GetFlagsOffset()) & 1;
}

void FNetworkObjectInfo::SetPendingNetUpdate(bool bInPendingNetUpdate)
{
    auto& Flags = *(uint32*)((uint8*)this + GetFlagsOffset());
    Flags = (Flags & ~1u) | (bInPendingNetUpdate ? 1u : 0u);
}

bool FNetworkObjectInfo::GetForceRelevantNextUpdate() const
{
    return *(const uint32*)((const uint8*)this + GetFlagsOffset()) & 2;
}

void FNetworkObjectInfo::SetForceRelevantNextUpdate(bool bInForceRelevantNextUpdate)
{
    auto& Flags = *(uint32*)((uint8*)this + GetFlagsOffset());
    Flags = (Flags & ~2u) | (bInForceRelevantNextUpdate ? 2u : 0u);
}

TSet<TWeakObjectPtr<UNetConnection>>& FNetworkObjectInfo::GetDormantConnections()
{
    return *(TSet<TWeakObjectPtr<UNetConnection>>*)((uint8*)this + GetFlagsOffset() + 0x8);
}

TSet<TWeakObjectPtr<UNetConnection>>& FNetworkObjectInfo::GetRecentlyDormantConnections()
{
    return *(TSet<TWeakObjectPtr<UNetConnection>>*)((uint8*)this + GetFlagsOffset() + 0x8 + sizeof(TSet<TWeakObjectPtr<UNetConnection>>));
}
