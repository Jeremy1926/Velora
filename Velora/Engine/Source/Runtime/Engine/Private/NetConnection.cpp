#include "pch.h"
#include "Engine/NetConnection.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static auto IsNetReadySlot = Scanner::String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")).Start().Find("xor edx, edx").Find("call [*]").Slot(TEXT("UNetConnection::IsNetReady"));

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
    static auto State = Scanner::String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")).Start().Find("cmp dword ptr [*], 0x1").Disp(TEXT("UNetConnection::State"));

    return *(EConnectionState*)((uint8_t*)this + State);
}

double UNetConnection::GetLastReceiveTime() const
{
    static auto LastReceiveTime = GetClass()->FindPropertyByName(FName(TEXT("LastReceiveTime")));

    return *LastReceiveTime->ContainerPtrToValuePtr<double>(this);
}

static int32_t TickCountOffset()
{
    static auto TickCount = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__NetConnection"));

        return Scanner::At(VTable[IsNetReadySlot / sizeof(void*)]).Find("mov [*], e*").Disp(TEXT("UNetConnection::QueuedBits")) + (int32_t)sizeof(int32);
    }();

    return TickCount;
}

int32 UNetConnection::GetTickCount() const
{
    return *(int32*)((uint8_t*)this + TickCountOffset());
}

void UNetConnection::SetTickCount(int32 InTickCount)
{
    *(int32*)((uint8_t*)this + TickCountOffset()) = InTickCount;
}

void UNetConnection::SetTimeSensitive(bool bInTimeSensitive)
{
    static auto TimeSensitive = Scanner::String(TEXT("UNetConnection::Tick: Connection TIMED OUT. Closing connection.")).Start().Find("cmp byte ptr [*+0x???], 0x0").Disp(TEXT("UNetConnection::TimeSensitive"));

    *(bool*)((uint8_t*)this + TimeSensitive) = bInTimeSensitive;
}

TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>& UNetConnection::GetActorChannels()
{
    static auto ActorChannels = []
    {
        auto Close = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start();
        auto Add = Close.Find("add rcx, 0x*");

        return Add.Get() ? (int32)Add.Imm(TEXT("UNetConnection::ActorChannels")) : Close.Find("mov rcx, [*+0x28]; call 0x*").Call().Find("lea rcx, [rbx+0x*]").Disp(TEXT("UNetConnection::ActorChannels"));
    }();

    return *(TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>*)((uint8_t*)this + ActorChannels);
}

TMap<TWeakObjectPtr<AActor>, double, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, double>>& UNetConnection::GetLastUpdateTimes()
{
    static auto LastUpdateTimes = Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Find("cvtps2pd *").Find("movsd [*], xmm*").Last("lea rcx, [*+0x*]; call 0x*; movsxd rax, *; cmp eax, *").Disp(TEXT("UNetConnection::LastUpdateTimes"));

    return *(TMap<TWeakObjectPtr<AActor>, double, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, double>>*)((uint8_t*)this + LastUpdateTimes);
}

TSet<FNetworkGUID>& UNetConnection::GetDestroyedStartupOrDormantActors()
{
    static auto DestroyedStartupOrDormantActors = []
    {
        auto Destroy = Scanner::String(TEXT("Adding actor NetGUID <%s> to new connection's destroy list"));
        auto Add = Destroy.Find("lea rcx, [*+0x*]; add r8, *");

        return (Add.Get() ? Add : Destroy.Find("add r8, *; lea rcx, [*+0x*]").Find("lea rcx, [*+0x*]")).Disp(TEXT("UNetConnection::DestroyedStartupOrDormantActors"));
    }();

    return *(TSet<FNetworkGUID>*)((uint8_t*)this + DestroyedStartupOrDormantActors);
}

TSet<FName>& UNetConnection::GetClientVisibleLevelNames()
{
    static auto ClientVisibleLevelNames = Scanner::String(TEXT("ServerUpdateLevelVisibility() Added '%s'")).Last("lea rcx, [*+0x*]; xor r9d, r9d").Disp(TEXT("UNetConnection::ClientVisibleLevelNames"));

    return *(TSet<FName>*)((uint8_t*)this + ClientVisibleLevelNames);
}

int32 UNetConnection::IsNetReady(bool Saturate)
{
    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(UNetConnection*, bool))(VTable + IsNetReadySlot))(this, Saturate);
}

UChannel* UNetConnection::CreateChannel(EChannelType Type, bool bOpenedLocally, int32 ChannelIndex)
{
    static auto CreateChannel = Scanner::Function<UChannel* (*)(UNetConnection*, EChannelType, bool, int32)>(Scanner::String(TEXT("Created channel %i of type %i")).Start().Get(TEXT("UNetConnection::CreateChannel")));

    return CreateChannel(this, Type, bOpenedLocally, ChannelIndex);
}

void UNetConnection::FlushDormancy(AActor* Actor)
{
    static auto FlushDormancy = []
    {
        auto Function = Scanner::String(TEXT("FlushDormancy: %s. Connection: %s")).Start().Get(TEXT("UNetConnection::FlushDormancy"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__NetConnection"));

        return Scanner::Slot(TEXT("UNetConnection::FlushDormancy"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == Function; });
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(UNetConnection*, AActor*))(VTable + FlushDormancy))(this, Actor);
}
