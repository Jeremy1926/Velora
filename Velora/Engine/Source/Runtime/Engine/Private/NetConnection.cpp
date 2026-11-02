#include "pch.h"
#include "Engine/NetConnection.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

TArray<UChildConnection*>& UNetConnection::GetChildren()
{
    static auto Children = GetClass()->FindPropertyByName(FName(TEXT("Children")));

    return *Children->ContainerPtrToValuePtr<TArray<UChildConnection*>>(this);
}

UNetDriver* UNetConnection::GetDriver() const
{
    static auto Driver = GetClass()->FindPropertyByName(FName(TEXT("Driver")));

    return *Driver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

UPackageMap* UNetConnection::GetPackageMap() const
{
    static auto PackageMap = GetClass()->FindPropertyByName(FName(TEXT("PackageMap")));

    return *PackageMap->ContainerPtrToValuePtr<UPackageMap*>(this);
}

TArray<AActor*>& UNetConnection::GetSentTemporaries()
{
    static auto SentTemporaries = GetClass()->FindPropertyByName(FName(TEXT("SentTemporaries")));

    return *SentTemporaries->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

AActor* UNetConnection::GetViewTarget() const
{
    static auto ViewTarget = GetClass()->FindPropertyByName(FName(TEXT("ViewTarget")));

    return *ViewTarget->ContainerPtrToValuePtr<AActor*>(this);
}

void UNetConnection::SetViewTarget(AActor* InViewTarget)
{
    static auto ViewTarget = GetClass()->FindPropertyByName(FName(TEXT("ViewTarget")));

    *ViewTarget->ContainerPtrToValuePtr<AActor*>(this) = InViewTarget;
}

AActor* UNetConnection::GetOwningActor() const
{
    static auto OwningActor = GetClass()->FindPropertyByName(FName(TEXT("OwningActor")));

    return *OwningActor->ContainerPtrToValuePtr<AActor*>(this);
}

bool UNetConnection::IsReplay() const
{
    static auto InternalAck = GetClass()->FindPropertyByName(FName(TEXT("InternalAck")));

    return (*InternalAck->ContainerPtrToValuePtr<uint32>(this) >> 2) & 1;
}

AActor*& UNetConnection::GetRepContextActor()
{
    static auto RepContextActor = Anchor::Offset(TEXT("UNetConnection::RepContextActor"))
        .Pattern("48 8B 01 48 85 C0 74 13 48 83 A0 ? ? ? ? 00 48 8B 01 48 83 A0 ? ? ? ? 00 C3")
        .Find("and qword ptr [rax+0x*], 0x0")
        .Disp();

    return *(AActor**)((uint8*)this + RepContextActor);
}

ULevel*& UNetConnection::GetRepContextLevel()
{
    static auto RepContextLevel = Anchor::Offset(TEXT("UNetConnection::RepContextLevel"))
        .Pattern("48 8B 01 48 85 C0 74 13 48 83 A0 ? ? ? ? 00 48 8B 01 48 83 A0 ? ? ? ? 00 C3")
        .Find("and qword ptr [rax+0x*], 0x0")
        .Find("and qword ptr [rax+0x*], 0x0")
        .Disp();

    return *(ULevel**)((uint8*)this + RepContextLevel);
}

bool UNetConnection::GetPendingCloseDueToReplicationFailure()
{
    return *((uint8*)&GetRepContextLevel() - 0xA2) != 0;
}

bool UNetConnection::GetInternalAck() const
{
    static auto InternalAck = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("InternalAck")));

    return InternalAck->GetPropertyValue_InContainer(this);
}

EConnectionState UNetConnection::GetState() const
{
    static auto State = Anchor::Offset(TEXT("UNetConnection::State"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("cmp dword ptr [*], 0x1")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov eax, [r*+0x*]; cmp eax, 0x4; jz *; cmp eax, 0x1")
        .Disp();

    return *(EConnectionState*)((uint8_t*)this + State);
}

double UNetConnection::GetLastReceiveTime() const
{
    static auto LastReceiveTime = GetClass()->FindPropertyByName(FName(TEXT("LastReceiveTime")));

    return *LastReceiveTime->ContainerPtrToValuePtr<double>(this);
}

int32& UNetConnection::GetQueuedBits()
{
    static auto QueuedBits = Anchor::Offset(TEXT("UNetConnection::QueuedBits"))
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov r8d, [rcx+0x*]; mov eax, 0x1; shr r8d, 0x2; mov r10b, dl; test al, r8b; jnz *; xor edx, edx")
        .Find("lea r8, [rcx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .LogSlot(TEXT("UNetConnection::IsNetReady"))
        .Entry(TEXT("/Script/Engine.Default__NetConnection"))
        .Find("mov [*], e*")
        .Or()
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov eax, [rcx+0x*]; shr eax, 0x2; test al, 0x1; jnz *; test dl, dl; jz *; mov eax, [rcx+0x*]; neg eax; mov [rcx+0x*], eax")
        .Find("mov [*], e*")
        .Disp();

    return *(int32*)((uint8_t*)this + QueuedBits);
}

int32 UNetConnection::GetTickCount() const
{
    static auto QueuedBits = Anchor::Offset(TEXT("UNetConnection::QueuedBits"))
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov r8d, [rcx+0x*]; mov eax, 0x1; shr r8d, 0x2; mov r10b, dl; test al, r8b; jnz *; xor edx, edx")
        .Find("lea r8, [rcx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .LogSlot(TEXT("UNetConnection::IsNetReady"))
        .Entry(TEXT("/Script/Engine.Default__NetConnection"))
        .Find("mov [*], e*")
        .Or()
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov eax, [rcx+0x*]; shr eax, 0x2; test al, 0x1; jnz *; test dl, dl; jz *; mov eax, [rcx+0x*]; neg eax; mov [rcx+0x*], eax")
        .Find("mov [*], e*")
        .Disp();

    return *(int32*)((uint8_t*)this + QueuedBits + sizeof(int32));
}

void UNetConnection::SetTickCount(int32 InTickCount)
{
    static auto QueuedBits = Anchor::Offset(TEXT("UNetConnection::QueuedBits"))
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov r8d, [rcx+0x*]; mov eax, 0x1; shr r8d, 0x2; mov r10b, dl; test al, r8b; jnz *; xor edx, edx")
        .Find("lea r8, [rcx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .LogSlot(TEXT("UNetConnection::IsNetReady"))
        .Entry(TEXT("/Script/Engine.Default__NetConnection"))
        .Find("mov [*], e*")
        .Or()
        .VirtualMatch(TEXT("/Script/Engine.Default__NetConnection"), "sub rsp, 0x28; mov eax, [rcx+0x*]; shr eax, 0x2; test al, 0x1; jnz *; test dl, dl; jz *; mov eax, [rcx+0x*]; neg eax; mov [rcx+0x*], eax")
        .Find("mov [*], e*")
        .Disp();

    *(int32*)((uint8_t*)this + QueuedBits + sizeof(int32)) = InTickCount;
}

void UNetConnection::SetTimeSensitive(bool bInTimeSensitive)
{
    static auto TimeSensitive = Anchor::Offset(TEXT("UNetConnection::TimeSensitive"))
        .String(TEXT("UNetConnection::Tick: Connection TIMED OUT. Closing connection."))
        .Start()
        .Find("cmp byte ptr [*+0x???], 0x0")
        .Disp();

    *(bool*)((uint8_t*)this + TimeSensitive) = bInTimeSensitive;
}

TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>& UNetConnection::GetActorChannels()
{
    static auto ActorChannels = Anchor::Offset(TEXT("UNetConnection::ActorChannels"))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Find("add rcx, 0x*")
        .Or()
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Find("mov rcx, [*+0x28]; call 0x*")
        .Call()
        .Find("lea rcx, [rbx+0x*]")
        .Or()
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Find("mov rdx, [*+0x68]; call 0x*")
        .Call()
        .Find("lea rcx, [rbx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("lea rdx, [*]; mov rcx, *; call 0x*; mov *, [*]; mov *, rax; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rcx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("lea *, [*+0x*]; mov rcx, *; lea rdx, [*]; call 0x*; cmp dword ptr [*], 0xffffffff")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("lea rdx, [*]; mov rcx, *; call 0x*; mov *, rax; test rax, rax; jnz *")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rcx+0x*]")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("lea rbx, [*+0x*]; mov [*], rax; lea r8, [*]; shr rax, 0x20; mov rcx, rbx")
        .Get();

    return *(TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>*)((uint8_t*)this + ActorChannels);
}

TMap<TWeakObjectPtr<AActor>, double, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, double>>& UNetConnection::GetLastUpdateTimes()
{
    static auto LastUpdateTimes = Anchor::Offset(TEXT("UNetConnection::LastUpdateTimes"))
        .String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"))
        .Start()
        .Find("cvtps2pd *")
        .Find("movsd [*], xmm*")
        .Last("lea rcx, [*+0x*]; call 0x*; movsxd rax, *; cmp eax, *")
        .Disp();

    return *(TMap<TWeakObjectPtr<AActor>, double, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, double>>*)((uint8_t*)this + LastUpdateTimes);
}

TSet<FNetworkGUID>& UNetConnection::GetDestroyedStartupOrDormantActors()
{
    static auto DestroyedStartupOrDormantActors = Anchor::Offset(TEXT("UNetConnection::DestroyedStartupOrDormantActors"))
        .String(TEXT("Adding actor NetGUID <%s> to new connection's destroy list"))
        .Find("lea rcx, [*+0x*]; add r8, *")
        .Or()
        .String(TEXT("Adding actor NetGUID <%s> to new connection's destroy list"))
        .Find("add r8, *; lea rcx, [*+0x*]")
        .Find("lea rcx, [*+0x*]")
        .Disp();

    return *(TSet<FNetworkGUID>*)((uint8_t*)this + DestroyedStartupOrDormantActors);
}

TSet<FName>& UNetConnection::GetClientVisibleLevelNames()
{
    static auto ClientVisibleLevelNames = Anchor::Offset(TEXT("UNetConnection::ClientVisibleLevelNames"))
        .String(TEXT("ServerUpdateLevelVisibility() Added '%s'"))
        .Last("lea rcx, [*+0x*]; xor r9d, r9d")
        .Disp();

    return *(TSet<FName>*)((uint8_t*)this + ClientVisibleLevelNames);
}

int32 UNetConnection::IsNetReady(bool Saturate)
{
    static auto IsNetReady = Anchor::VirtualFunction<int32 (*)(UNetConnection*, bool)>(TEXT("UNetConnection::IsNetReady"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .Get();

    return IsNetReady(this, Saturate);
}

UChannel* UNetConnection::CreateChannel(EChannelType Type, bool bOpenedLocally, int32 ChannelIndex)
{
    static auto CreateChannel = Anchor::Function<UChannel* (*)(UNetConnection*, EChannelType, bool, int32)>(TEXT("UNetConnection::CreateChannel"))
        .String(TEXT("Created channel %i of type %i"))
        .Start()
        .Get();

    return CreateChannel(this, Type, bOpenedLocally, ChannelIndex);
}

UChannel* UNetConnection::CreateChannelByName(const FName& ChName, EChannelCreateFlags CreateFlags, int32 ChIndex)
{
    static auto CreateChannelByName = Anchor::Function<UChannel* (*)(UNetConnection*, const FName&, EChannelCreateFlags, int32)>(TEXT("UNetConnection::CreateChannelByName"))
        .String(TEXT("Created channel %i of type %s"))
        .Start()
        .Get();

    return CreateChannelByName(this, ChName, CreateFlags, ChIndex);
}

void UNetConnection::FlushDormancy(AActor* Actor)
{
    static auto FlushDormancy = Anchor::VirtualFunction<void (*)(UNetConnection*, AActor*)>(TEXT("UNetConnection::FlushDormancy"))
        .String(TEXT("FlushDormancy: %s. Connection: %s"))
        .Start()
        .Log(TEXT("UNetConnection::FlushDormancy"))
        .SlotIn(TEXT("/Script/Engine.Default__NetConnection"))
        .Get();

    FlushDormancy(this, Actor);
}

UNetConnection* UChildConnection::GetParent() const
{
    static auto Parent = GetClass()->FindPropertyByName(FName(TEXT("Parent")));

    return *Parent->ContainerPtrToValuePtr<UNetConnection*>(this);
}

TArray<UChannel*>& UNetConnection::GetChannels()
{
    static auto Channels = Anchor::Offset(TEXT("UNetConnection::Channels"))
        .String(TEXT("UNetConnection::Close: %s, Channels: %i, Time: %s"))
        .Find("mov rdx, [*+0x*]; cmp [rdx], *")
        .Or()
        .String(TEXT("UNetConnection::Close: %s, Channels: %i, Time: %s"))
        .Start()
        .Find("mov rdx, [*+0x*]; cmp [rdx], *")
        .Or()
        .String(TEXT("UNetConnection::Close: %s, Channels: %i, Time: %s"))
        .Find("mov rax, [*+0x*]; mov rcx, [rax]; test rcx, rcx; jz 0x*; mov rax, [rcx]; xor edx, edx; call [rax+0x*]")
        .Disp();

    return *(TArray<UChannel*>*)((uint8_t*)this + Channels);
}

bool UNetConnection::IsReplayReady() const
{
    static auto IsReplayReady = Anchor::VirtualFunction<bool (*)(const UNetConnection*)>(TEXT("UNetConnection::IsReplayReady"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x20)
        .Find("call [rax+0x*]")
        .SlotShift(TEXT("/Script/Engine.Default__NetConnection"), -0x8)
        .Get();

    return IsReplayReady(this);
}

void UNetConnection::Close()
{
    static auto Close = Anchor::Function<void (*)(UNetConnection*)>(TEXT("UNetConnection::Close"))
        .String(TEXT("UReplicationGraph_NotifyActorRenamed"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x18)
        .Find("call [rax+0x*]; lea rcx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("UReplicationGraph_NotifyActorRenamed"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x18)
        .Find("movdqu [rcx+0x*], xmm0")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("UReplicationGraph_NotifyActorRenamed"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x18)
        .Find("mov [rcx+0x*], rbx; mov [rcx+0x*], rbx")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x38)
        .Find("call [rax+0x*]; lea rcx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x38)
        .Find("movdqu [rcx+0x*], xmm0")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), 0x38)
        .Find("mov [rcx+0x*], rbx; mov [rcx+0x*], rbx")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+rax*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    Close(this);
}

UChildConnection* UNetConnection::GetUChildConnection()
{
    static auto GetUChildConnection = Anchor::VirtualFunction<UChildConnection* (*)(UNetConnection*)>(TEXT("UNetConnection::GetUChildConnection"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x20)
        .Find("call [rax+0x*]")
        .Get();

    return GetUChildConnection(this);
}

UReplicationConnectionDriver*& UNetConnection::GetReplicationConnectionDriver()
{
    static auto ReplicationConnectionDriver = Anchor::Offset(TEXT("UNetConnection::ReplicationConnectionDriver"))
        .String(TEXT("ReplicationDriverClass is null! Not using ReplicationDriver."))
        .Start()
        .CallSite()
        .Find("call 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rcx+0x*]")
        .Disp();

    return *(UReplicationConnectionDriver**)((uint8*)this + ReplicationConnectionDriver);
}
