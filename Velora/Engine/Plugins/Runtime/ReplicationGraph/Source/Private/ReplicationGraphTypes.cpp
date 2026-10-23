#include "pch.h"
#include "ReplicationGraphTypes.h"
#include "Engine/NetDriver.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
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

FVector FGlobalActorReplicationInfo::GetWorldLocation() const
{
    return TGameLayout<FVector>::Load((const uint8*)this + (UE::LWC::IsEnabled() ? offsetof(TGlobalActorReplicationInfoLayout<FVector3d>, WorldLocation) : offsetof(TGlobalActorReplicationInfoLayout<FVector>, WorldLocation)));
}

void FGlobalActorReplicationInfo::SetWorldLocation(const FVector& InWorldLocation)
{
    TGameLayout<FVector>::Store((uint8*)this + (UE::LWC::IsEnabled() ? offsetof(TGlobalActorReplicationInfoLayout<FVector3d>, WorldLocation) : offsetof(TGlobalActorReplicationInfoLayout<FVector>, WorldLocation)), InWorldLocation);
}

float FGlobalActorReplicationInfo::GetCullDistance() const
{
    return *(const float*)(GetSettings() + (FName::IsOutlineNumber() ? offsetof(TClassReplicationInfoLayout<NAME_INDEX>, CullDistance) : offsetof(TClassReplicationInfoLayout<FName>, CullDistance)));
}

uint32& FGlobalActorReplicationInfo::GetLastPreReplicationFrame()
{
    return *(uint32*)((uint8*)this + offsetof(TGlobalActorReplicationInfoLayout<FVector>, LastPreReplicationFrame));
}

uint32 FGlobalActorReplicationInfo::GetForceNetUpdateFrame() const
{
    return *(const uint32*)((const uint8*)this + offsetof(TGlobalActorReplicationInfoLayout<FVector>, ForceNetUpdateFrame));
}

bool FGlobalActorReplicationInfo::GetWantsToBeDormant() const
{
    return *((const uint8*)this + (UE::LWC::IsEnabled() ? offsetof(TGlobalActorReplicationInfoLayout<FVector3d>, bWantsToBeDormant) : offsetof(TGlobalActorReplicationInfoLayout<FVector>, bWantsToBeDormant))) != 0;
}

bool FGlobalActorReplicationInfo::GetSwapRolesOnReplicate() const
{
    return *((const uint8*)this + (UE::LWC::IsEnabled() ? offsetof(TGlobalActorReplicationInfoLayout<FVector3d>, bSwapRolesOnReplicate) : offsetof(TGlobalActorReplicationInfoLayout<FVector>, bSwapRolesOnReplicate))) != 0;
}

float FGlobalActorReplicationInfo::GetDistancePriorityScale() const
{
    return *(const float*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, DistancePriorityScale));
}

float FGlobalActorReplicationInfo::GetStarvationPriorityScale() const
{
    return *(const float*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, StarvationPriorityScale));
}

float FGlobalActorReplicationInfo::GetAccumulatedNetPriorityBias() const
{
    return *(const float*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, AccumulatedNetPriorityBias));
}

uint16 FGlobalActorReplicationInfo::GetActorChannelFrameTimeout() const
{
    return *(const uint16*)(GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, ActorChannelFrameTimeout));
}

TArray<AActor*>& FGlobalActorReplicationInfo::GetDependentActorList()
{
    static auto DependentActorList = Anchor::Offset(TEXT("FGlobalActorReplicationInfo::DependentActorList"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x10)
        .Find("call 0x*", 3)
        .Target()
        .Find("lea r14, [rdi+0x*]")
        .Disp();

    return *(TArray<AActor*>*)((uint8*)this + DependentActorList);
}

uint8* FGlobalActorReplicationInfo::GetFastSharedReplicationFunc()
{
    return (uint8*)GetSettings() + offsetof(TClassReplicationInfoLayout<FName>, FastSharedReplicationFunc);
}

FName FGlobalActorReplicationInfo::GetFastSharedReplicationFuncName() const
{
    return TGameLayout<FName>::Load(GetSettings() + (FName::IsOutlineNumber() ? offsetof(TClassReplicationInfoLayout<NAME_INDEX>, FastSharedReplicationFuncName) : offsetof(TClassReplicationInfoLayout<FName>, FastSharedReplicationFuncName)));
}

FFastSharedReplicationInfo*& FGlobalActorReplicationInfo::GetFastSharedReplicationInfo()
{
    static auto FastSharedReplicationInfo = Anchor::Offset(TEXT("FGlobalActorReplicationInfo::FastSharedReplicationInfo"))
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
        .Find("call 0x*")
        .Target()
        .Find("mov rdi, [rbx+0x*]; test rdi, rdi; jz 0x*; lea rcx, [rdi+0x8]; call 0x*")
        .Disp();

    return *(FFastSharedReplicationInfo**)((uint8*)this + FastSharedReplicationInfo);
}

FGlobalActorReplicationInfo* FGlobalActorReplicationInfoMap::Find(AActor* const& Actor)
{
    auto InfoPtr = ActorMap.Find(Actor);

    return InfoPtr ? *InfoPtr : nullptr;
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

TArray<FNetViewer, TInlineAllocator<4>>::TArray()
{
    GetSecondaryData() = nullptr;
    GetArrayNum() = 0;
    GetArrayMax() = 4;
}

TArray<FNetViewer, TInlineAllocator<4>>::~TArray()
{
    if (GetSecondaryData())
        FMemory::Free(GetSecondaryData());
}

int32 TArray<FNetViewer, TInlineAllocator<4>>::Num() const
{
    return GetArrayNum();
}

FNetViewer& TArray<FNetViewer, TInlineAllocator<4>>::operator[](int32 Index) const
{
    return *(FNetViewer*)(GetData() + Index * FNetViewer::GetTypeSize());
}

TRuntimeSizedArray<FNetViewer>::TIterator TArray<FNetViewer, TInlineAllocator<4>>::begin() const
{
    return TRuntimeSizedArray<FNetViewer>::TIterator(GetData());
}

TRuntimeSizedArray<FNetViewer>::TIterator TArray<FNetViewer, TInlineAllocator<4>>::end() const
{
    return TRuntimeSizedArray<FNetViewer>::TIterator(GetData() + GetArrayNum() * FNetViewer::GetTypeSize());
}

FNetViewer& TArray<FNetViewer, TInlineAllocator<4>>::Emplace(UNetConnection* InConnection, float DeltaSeconds)
{
    const int32 Index = GetArrayNum();

    if (Index + 1 > GetArrayMax())
    {
        const int32 NewMax = DefaultCalculateSlackGrow(Index + 1, GetArrayMax(), FNetViewer::GetTypeSize(), true);
        const bool bWasInline = GetSecondaryData() == nullptr;

        GetSecondaryData() = (uint8*)FMemory::Realloc(GetSecondaryData(), NewMax * FNetViewer::GetTypeSize());

        if (bWasInline)
            memcpy(GetSecondaryData(), Storage, Index * FNetViewer::GetTypeSize());

        GetArrayMax() = NewMax;
    }

    GetArrayNum() = Index + 1;

    return *new(GetData() + Index * FNetViewer::GetTypeSize()) FNetViewer(InConnection, DeltaSeconds);
}

uint8*& TArray<FNetViewer, TInlineAllocator<4>>::GetSecondaryData() const
{
    return *(uint8**)((uint8*)Storage + 4 * FNetViewer::GetTypeSize());
}

int32& TArray<FNetViewer, TInlineAllocator<4>>::GetArrayNum() const
{
    return *(int32*)((uint8*)Storage + 4 * FNetViewer::GetTypeSize() + sizeof(uint8*));
}

int32& TArray<FNetViewer, TInlineAllocator<4>>::GetArrayMax() const
{
    return *(int32*)((uint8*)Storage + 4 * FNetViewer::GetTypeSize() + sizeof(uint8*) + sizeof(int32));
}

uint8* TArray<FNetViewer, TInlineAllocator<4>>::GetData() const
{
    return GetSecondaryData() ? GetSecondaryData() : (uint8*)Storage;
}

bool FActorRepListRefView::ConditionalAdd(AActor* NewElement)
{
    if (IsActorValidForReplicationGather(NewElement))
    {
        RepList.Add(NewElement);

        return true;
    }

    return false;
}

bool IsActorValidForReplication(const AActor* In)
{
    return !In->IsPendingKillPending() && !(In->GetFlags() & 0x60000000) && !GUObjectArray.IndexToObject((int32)In->GetUniqueID())->IsUnreachable();
}

bool IsActorValidForReplicationGather(const AActor* In)
{
    return In && IsActorValidForReplication(In) && In->GetIsReplicated() && !In->GetTearOff() && !(In->GetNetDormancy() == DORM_Initial && In->IsNetStartupActor());
}

int32 FConnectionGatherActorListParameters::GetNumViewers() const
{
    return *(const int32*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*));
}

const FNetViewer& FConnectionGatherActorListParameters::GetViewer(int32 Index) const
{
    auto SecondaryData = *(const uint8* const*)((const uint8*)this + 4 * FNetViewer::GetTypeSize());

    return *(const FNetViewer*)((SecondaryData ? SecondaryData : (const uint8*)this) + Index * FNetViewer::GetTypeSize());
}

UNetReplicationGraphConnection* FConnectionGatherActorListParameters::GetConnectionManager() const
{
    return *(UNetReplicationGraphConnection* const*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*) + 2 * sizeof(int32));
}

uint32 FConnectionGatherActorListParameters::GetReplicationFrameNum() const
{
    return *(const uint32*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*) + 2 * sizeof(int32) + sizeof(void*));
}

FGatheredReplicationActorLists* FConnectionGatherActorListParameters::GetOutGatheredReplicationLists() const
{
    return *(FGatheredReplicationActorLists* const*)((const uint8*)this + 4 * FNetViewer::GetTypeSize() + sizeof(void*) + 2 * sizeof(int32) + 2 * sizeof(void*));
}

bool FConnectionGatherActorListParameters::CheckClientVisibilityForLevel(const FName& StreamingLevelName) const
{
    static auto CheckClientVisibilityForLevel = Anchor::Function<bool (*)(const FConnectionGatherActorListParameters*, const FName&)>(TEXT("FConnectionGatherActorListParameters::CheckClientVisibilityForLevel"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    return CheckClientVisibilityForLevel(this, StreamingLevelName);
}

void FStreamingLevelActorListCollection::Gather(const FConnectionGatherActorListParameters& Params)
{
    static auto Gather = Anchor::Function<void (*)(FStreamingLevelActorListCollection*, const FConnectionGatherActorListParameters&)>(TEXT("FStreamingLevelActorListCollection::Gather"))
        .String(TEXT("Child: %s"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraphNode_ActorList"), -0x18)
        .Find("call 0x*", 2)
        .Target()
        .Get();

    Gather(this, Params);
}

void FStreamingLevelActorListCollection::DeepCopyFrom(const FStreamingLevelActorListCollection& Source)
{
    StreamingLevelLists.Reset();

    for (const auto& StreamingLevel : Source.StreamingLevelLists)
    {
        if (StreamingLevel.ReplicationActorList.RepList.Num() > 0)
        {
            auto NewStreamingLevel = new(StreamingLevelLists) FStreamingLevelActors(TGameLayout<FName>::Load(&StreamingLevel.StreamingLevelName));
            NewStreamingLevel->ReplicationActorList.RepList = StreamingLevel.ReplicationActorList.RepList;
        }
    }
}

FNewReplicatedActorInfo::FNewReplicatedActorInfo(AActor* InActor)
    : Actor(InActor)
    , StreamingLevelName(NAME_None)
    , Class(InActor->GetClass())
{
    TGameLayout<FName>::Store(&StreamingLevelName, GetStreamingLevelNameOfActor(InActor));
}

FName FNewReplicatedActorInfo::GetStreamingLevelNameOfActor(const AActor* Actor)
{
    ULevel* Level = Actor ? Cast<ULevel>(Actor->GetOuter()) : nullptr;

    return (Level && !Level->IsPersistentLevel()) ? Level->GetOutermost()->GetFName() : FName(NAME_None);
}
