#include "pch.h"
#include "ReplicationGraphTypes.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

const uint8* FGlobalActorReplicationInfo::GetSettings() const
{
    return (const uint8*)this + (UE::LWC::IsEnabled() ? offsetof(TGlobalActorReplicationInfoLayout<FVector3d>, Settings) : offsetof(TGlobalActorReplicationInfoLayout<FVector>, Settings));
}

uint16 FGlobalActorReplicationInfo::GetReplicationPeriodFrame() const
{
    return *(const uint16*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, ReplicationPeriodFrame));
}

uint16 FGlobalActorReplicationInfo::GetFastPathReplicationPeriodFrame() const
{
    return *(const uint16*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, FastPath_ReplicationPeriodFrame));
}

float FGlobalActorReplicationInfo::GetCullDistanceSquared() const
{
    return *(const float*)(GetSettings() + (FName::IsOutlineNumber() ? offsetof(TClassReplicationInfoLayout<NAME_INDEX>, CullDistanceSquared) : offsetof(TClassReplicationInfoLayout<FName>, CullDistanceSquared)));
}

FGlobalActorReplicationInfo& FGlobalActorReplicationInfoMap::Get(AActor* const& Actor)
{
    static auto Get = Anchor::Function<FGlobalActorReplicationInfo& (*)(FGlobalActorReplicationInfoMap*, AActor* const&)>(TEXT("FGlobalActorReplicationInfoMap::Get"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"), [](const Anchor::FCode& Entry) { return Entry.Has("mov ecx, 0x30; call 0x*; mov *, rax; test rax, rax; jz 0x*; mov rcx, [*+0x*]"); })
        .Find("mov ecx, 0x30; call 0x*")
        .Find("mov rcx, [*+0x*]; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return Get(this, Actor);
}

void FGlobalActorReplicationInfoMap::ResetActorMap()
{
    static auto DestructItem = Anchor::Function<void (*)(void*)>(TEXT("TPair<FActorRepListType, TUniquePtr<FGlobalActorReplicationInfo>>::~TPair"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x10)
        .Find("lea rcx, [*+0x*]; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("pop rdi; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("lea rcx, [rax+r8*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!ActorMap.Num())
        return;

    for (auto& Pair : ActorMap)
        DestructItem((void*)&Pair);

    ActorMap.Reset();
}

FConnectionReplicationActorInfo& FPerConnectionActorInfoMap::FindOrAdd(AActor* const& Actor)
{
    static auto ReferenceControllerVTable = Anchor::Data<void*>(TEXT("SharedPointerInternals::TReferenceControllerWithDeleter<FConnectionReplicationActorInfo>::vftable"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevantForSquad"), [](const Anchor::FCode& Entry) { return Entry.Has("mov ecx, 0x30; call 0x*; mov *, rax; test rax, rax; jz 0x*; mov rcx, [*+0x*]"); })
        .Find("mov ecx, 0x30; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    if (auto ValuePtr = ActorMap.Find(Actor))
        return *ValuePtr->Get();

    struct FReferenceController
    {
        void* VTable;
        int32 SharedReferenceCount;
        int32 WeakReferenceCount;
        FConnectionReplicationActorInfo* Object;
    };

    auto NewInfo = new(FMemory::Realloc(nullptr, sizeof(FConnectionReplicationActorInfo))) FConnectionReplicationActorInfo(GlobalMap->Get(Actor));
    auto ReferenceController = new(FMemory::Realloc(nullptr, sizeof(FReferenceController), 0x10)) FReferenceController{ ReferenceControllerVTable, 1, 1, NewInfo };

    ActorMap.Add(Actor, TSharedPtr<FConnectionReplicationActorInfo, ESPMode::ThreadSafe>(NewInfo, (SharedPointerInternals::FReferenceControllerBase*)ReferenceController));

    return *NewInfo;
}

void FPerConnectionActorInfoMap::AddChannel(AActor* const& Actor, UActorChannel* Channel)
{
    if (auto ValuePtr = ActorMap.Find(Actor))
        ChannelMap.Add(Channel, *ValuePtr);
}

void FPerConnectionActorInfoMap::RemoveChannel(UActorChannel* Channel)
{
    ChannelMap.Remove(Channel);
}

FCachedDestructInfo::FCachedDestructInfo(FActorDestructionInfo* InDestructionInfo)
    : DestructionInfo(InDestructionInfo)
{
    TGameLayout<FVector>::Store(this + 1, InDestructionInfo->GetDestroyedPosition());
}

bool IsActorValidForReplication(const AActor* In)
{
    return !In->IsPendingKillPending() && !(In->GetFlags() & 0x60000000) && !GUObjectArray.IndexToObject((int32)In->GetUniqueID())->IsUnreachable();
}

UNetReplicationGraphConnection* FConnectionGatherActorListParameters::GetConnectionManager() const
{
    return *(UNetReplicationGraphConnection* const*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*) + 2 * sizeof(int32));
}

FGatheredReplicationActorLists* FConnectionGatherActorListParameters::GetOutGatheredReplicationLists() const
{
    return *(FGatheredReplicationActorLists* const*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*) + 2 * sizeof(int32) + 2 * sizeof(void*));
}
