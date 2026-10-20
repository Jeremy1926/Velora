#include "pch.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Net/DataBunch.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UNetConnection* UChannel::GetConnection() const
{
    static auto Connection = GetClass()->FindPropertyByName(FName(TEXT("Connection")));

    return *Connection->ContainerPtrToValuePtr<UNetConnection*>(this);
}

bool UChannel::GetClosing() const
{
    static auto Offset = Anchor::Offset(TEXT("UChannel::Closing"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov e*, [rcx+0x*]; test cl, 0x*")
        .Or()
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("test byte ptr [*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("UChannel::Closing"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov e*, [rcx+0x*]; test cl, 0x*")
        .Find("test cl, 0x*")
        .Or()
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("test byte ptr [*], 0x*")
        .Value();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetDormant() const
{
    static auto Offset = Anchor::Offset(TEXT("UChannel::Dormant"))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Find("test byte ptr [*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("UChannel::Dormant"))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Find("test byte ptr [*], 0x*")
        .Value();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetPendingDormancy() const
{
    static auto Offset = Anchor::Offset(TEXT("UChannel::bPendingDormancy"))
        .String(TEXT("StartBecomingDormant: %s"))
        .Start()
        .Find("or dword ptr [*], 0x*")
        .Or()
        .String(TEXT("StartBecomingDormant: %s"))
        .Start()
        .Find("bts e*, 0x*")
        .Find("mov [*], e*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("UChannel::bPendingDormancy"))
        .String(TEXT("StartBecomingDormant: %s"))
        .Start()
        .Find("or dword ptr [*], 0x*")
        .Or()
        .String(TEXT("StartBecomingDormant: %s"))
        .Start()
        .Find("bts e*, 0x*")
        .Mask();

    return *(uint32_t*)((uint8_t*)this + Offset) & Mask;
}

int32 UChannel::GetNumOutRec() const
{
    static auto NumOutRec = Anchor::Offset(TEXT("UChannel::NumOutRec"))
        .String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"))
        .Start()
        .Find("jnb *; cmp *")
        .Find("cmp *")
        .Disp();

    return *(int32*)((uint8_t*)this + NumOutRec);
}

int32 UChannel::IsNetReady(bool Saturate)
{
    if (GetNumOutRec() >= 256 - 1)
    {
        return 0;
    }

    return GetConnection()->IsNetReady(Saturate);
}

FPacketIdRange UChannel::SendBunch(FOutBunch* Bunch, bool Merge)
{
    static auto SendBunch = Anchor::VirtualFunction<FPacketIdRange* (*)(UChannel*, FPacketIdRange*, FOutBunch*, bool)>(TEXT("UChannel::SendBunch"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("call 0x*")
        .Find("call [*]")
        .Get();

    FPacketIdRange Result;
    SendBunch(this, &Result, Bunch, Merge);

    return Result;
}

AActor* UActorChannel::GetActor() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}

double UActorChannel::GetRelevantTime() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *(double*)((uint8_t*)Actor->ContainerPtrToValuePtr<AActor*>(this) + sizeof(AActor*) + sizeof(FNetworkGUID) + sizeof(float));
}

void UActorChannel::SetRelevantTime(double InRelevantTime)
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    *(double*)((uint8_t*)Actor->ContainerPtrToValuePtr<AActor*>(this) + sizeof(AActor*) + sizeof(FNetworkGUID) + sizeof(float)) = InRelevantTime;
}

double UActorChannel::GetLastUpdateTime() const
{
    static auto LastUpdateTime = Anchor::Offset(TEXT("UActorChannel::LastUpdateTime"))
        .String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"))
        .Start()
        .Find("cvtps2pd *")
        .Find("movsd [*], xmm*")
        .Is("movsd [*+0x*], xmm*")
        .Optional()
        .Disp();

    if (LastUpdateTime)
        return *(double*)((uint8_t*)this + LastUpdateTime);

    return GetConnection()->GetLastUpdateTimes().FindRef(GetActor());
}

void UActorChannel::Close()
{
    static auto Close = Anchor::Function<void (*)(UActorChannel*)>(TEXT("UActorChannel::Close"))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s"))
        .Start()
        .Get();

    Close(this);
}

void UActorChannel::StartBecomingDormant()
{
    static auto StartBecomingDormant = Anchor::Function<void (*)(UActorChannel*)>(TEXT("UActorChannel::StartBecomingDormant"))
        .String(TEXT("StartBecomingDormant: %s"))
        .Start()
        .Get();

    StartBecomingDormant(this);
}

bool UActorChannel::ReplicateActor()
{
    static auto ReplicateActor = Anchor::Function<bool (*)(UActorChannel*)>(TEXT("UActorChannel::ReplicateActor"))
        .String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"))
        .Start()
        .Get();

    return ReplicateActor(this);
}

void UActorChannel::SetChannelActor(AActor* InActor)
{
    static auto SetChannelActor = Anchor::Function<void (*)(UActorChannel*, AActor*)>(TEXT("UActorChannel::SetChannelActor"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .FindCallToString(TEXT("Created channel %i of type %i"))
        .Find("call 0x*")
        .Target()
        .Get();

    SetChannelActor(this, InActor);
}

void UActorChannel::SetChannelActorForDestroy(FActorDestructionInfo* DestructInfo)
{
    if (!GetClosing() && (GetConnection()->GetState() == USOCK_Open || GetConnection()->GetState() == USOCK_Pending))
    {
        auto CloseBunch = new FOutBunch(this, 1);
        CloseBunch->SetReliable(1);
        CloseBunch->SetDormant(0);

        GetConnection()->GetPackageMap()->WriteObject(*(FArchive*)CloseBunch, DestructInfo->GetObjOuter().Get(), DestructInfo->GetNetGUID(), DestructInfo->GetPathName());

        SendBunch(CloseBunch, 0);

        delete CloseBunch;
    }
}
