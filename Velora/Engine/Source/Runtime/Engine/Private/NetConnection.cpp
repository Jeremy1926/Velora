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
        .Disp();

    return *(EConnectionState*)((uint8_t*)this + State);
}

double UNetConnection::GetLastReceiveTime() const
{
    static auto LastReceiveTime = GetClass()->FindPropertyByName(FName(TEXT("LastReceiveTime")));

    return *LastReceiveTime->ContainerPtrToValuePtr<double>(this);
}

int32 UNetConnection::GetTickCount() const
{
    static auto QueuedBits = Anchor::Offset(TEXT("UNetConnection::QueuedBits"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .LogSlot(TEXT("UNetConnection::IsNetReady"))
        .Entry(TEXT("/Script/Engine.Default__NetConnection"))
        .Find("mov [*], e*")
        .Disp();

    return *(int32*)((uint8_t*)this + QueuedBits + sizeof(int32));
}

void UNetConnection::SetTickCount(int32 InTickCount)
{
    static auto QueuedBits = Anchor::Offset(TEXT("UNetConnection::QueuedBits"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("xor edx, edx")
        .Find("call [*]")
        .LogSlot(TEXT("UNetConnection::IsNetReady"))
        .Entry(TEXT("/Script/Engine.Default__NetConnection"))
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
