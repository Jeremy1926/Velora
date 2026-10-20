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
    static auto Close = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start();
    static auto Load = Close.Find("mov e*, [rcx+0x*]; test cl, 0x*");
    static auto Closing = Load.Get() ? Load.Find("test cl, 0x*") : Close.Find("test byte ptr [*], 0x*");
    static auto Offset = (Load.Get() ? Load : Closing).Disp(TEXT("UChannel::Closing"));
    static auto Mask = Closing.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetDormant() const
{
    static auto Dormant = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("test byte ptr [*], 0x*");
    static auto Offset = Dormant.Disp(TEXT("UChannel::Dormant"));
    static auto Mask = Dormant.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetPendingDormancy() const
{
    static auto StartBecomingDormant = Scanner::String(TEXT("StartBecomingDormant: %s")).Start();
    static auto bPendingDormancy = StartBecomingDormant.Find("or dword ptr [*], 0x*");
    static auto Offset = (bPendingDormancy.Get() ? bPendingDormancy : StartBecomingDormant.Find("bts e*, 0x*").Find("mov [*], e*")).Disp(TEXT("UChannel::bPendingDormancy"));
    static auto Mask = bPendingDormancy.Get() ? bPendingDormancy.Imm() : 1ll << StartBecomingDormant.Find("bts e*, 0x*").Imm();

    return *(uint32_t*)((uint8_t*)this + Offset) & Mask;
}

int32 UChannel::GetNumOutRec() const
{
    static auto NumOutRec = Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Find("jnb *; cmp *").Find("cmp *").Disp(TEXT("UChannel::NumOutRec"));

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
    static auto SendBunch = Anchor::Slot(TEXT("UChannel::SendBunch"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("call 0x*")
        .Find("call [*]")
        .Get();

    FPacketIdRange Result;
    Scanner::Virtual<FPacketIdRange* (*)(UChannel*, FPacketIdRange*, FOutBunch*, bool)>(this, SendBunch)(this, &Result, Bunch, Merge);

    return Result;
}

AActor* UActorChannel::GetActor() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}


static int32 GetRelevantTimeOffset(const UActorChannel* Channel)
{
    static auto RelevantTime = Channel->GetClass()->FindPropertyByName(FName(TEXT("Actor")))->GetOffset_ForInternal() + (int32)(sizeof(AActor*) + sizeof(FNetworkGUID) + sizeof(float));

    return RelevantTime;
}

double UActorChannel::GetRelevantTime() const
{
    return *(double*)((uint8_t*)this + GetRelevantTimeOffset(this));
}

void UActorChannel::SetRelevantTime(double InRelevantTime)
{
    *(double*)((uint8_t*)this + GetRelevantTimeOffset(this)) = InRelevantTime;
}

double UActorChannel::GetLastUpdateTime() const
{
    static auto LastUpdateTimeStore = Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Find("cvtps2pd *").Find("movsd [*], xmm*");
    static auto LastUpdateTime = LastUpdateTimeStore.Is("movsd [*+0x*], xmm*") ? LastUpdateTimeStore.Disp(TEXT("UActorChannel::LastUpdateTime")) : 0;

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
        .FindCall(Scanner::String(TEXT("Created channel %i of type %i")).Start().Get())
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
