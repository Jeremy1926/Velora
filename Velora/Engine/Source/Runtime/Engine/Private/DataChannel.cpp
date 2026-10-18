#include "pch.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Net/DataBunch.h"
#include "UObject/UnrealType.h"
#include "Building/BuildingActor.h"
#include "Scanner.h"
#include "Detour.h"

static auto SetChannelActor = DetourCall(TEXT("UActorChannel::SetChannelActor"), Scanner::String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")).Start().Find("mov edx, 0x2").Find("call 0x*").Find("call 0x*").Get(), &UActorChannel::SetChannelActor);

UNetConnection* UChannel::GetConnection() const
{
    static auto Connection = GetClass()->FindPropertyByName(FName(TEXT("Connection")));

    return *Connection->ContainerPtrToValuePtr<UNetConnection*>(this);
}

bool UChannel::GetClosing() const
{
    static auto Closing = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start().Find("test byte ptr [*], 0x*");
    static auto Offset = Closing.Disp();
    static auto Mask = Closing.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetDormant() const
{
    static auto Dormant = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("test byte ptr [*], 0x*");
    static auto Offset = Dormant.Disp();
    static auto Mask = Dormant.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

bool UChannel::GetPendingDormancy() const
{
    static auto bPendingDormancy = Scanner::String(TEXT("StartBecomingDormant: %s")).Start().Find("or dword ptr [*], 0x*");
    static auto Offset = bPendingDormancy.Disp();
    static auto Mask = bPendingDormancy.Imm();

    return *(uint32_t*)((uint8_t*)this + Offset) & Mask;
}

int32 UChannel::GetNumOutRec() const
{
    static auto NumOutRec = Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Find("cmp dword ptr [*], 0x0").Disp();

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
    static auto SendBunch = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start().Find("mov r8b, 0x1").Find("call 0x*").Find("call [*]").Disp();

    auto VTable = *(uint8_t**)this;
    FPacketIdRange Result;

    return *(*(FPacketIdRange* (**)(UChannel*, FPacketIdRange*, FOutBunch*, bool))(VTable + SendBunch))(this, &Result, Bunch, Merge);
}

AActor* UActorChannel::GetActor() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}

static auto LastUpdateTime = Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Find("cvtps2pd *").Find("movsd [*], xmm*").Disp();

double UActorChannel::GetRelevantTime() const
{
    return *(double*)((uint8_t*)this + LastUpdateTime - sizeof(double));
}

void UActorChannel::SetRelevantTime(double InRelevantTime)
{
    *(double*)((uint8_t*)this + LastUpdateTime - sizeof(double)) = InRelevantTime;
}

double UActorChannel::GetLastUpdateTime() const
{
    return *(double*)((uint8_t*)this + LastUpdateTime);
}

void UActorChannel::Close()
{
    static auto Close = (void (*)(UActorChannel*))Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Get();

    Close(this);
}

void UActorChannel::StartBecomingDormant()
{
    static auto StartBecomingDormant = (void (*)(UActorChannel*))Scanner::String(TEXT("StartBecomingDormant: %s")).Start().Get();

    StartBecomingDormant(this);
}

bool UActorChannel::ReplicateActor()
{
    static auto ReplicateActor = (bool (*)(UActorChannel*))Scanner::String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")).Start().Get();

    return ReplicateActor(this);
}

void UActorChannel::SetChannelActor(AActor* InActor)
{
    (this->*::SetChannelActor)(InActor);

    if (!InActor->IsNetStartupActor() && (InActor->IsA(ABuildingActor::StaticClass()) || InActor->GetAlwaysRelevant()))
    {
        APlayerController* PlayerController = GetConnection()->GetPlayerController();
        AActor* ViewTarget = GetConnection()->GetViewTarget();
        ABuildingActor* BuildingActor = InActor->IsA(ABuildingActor::StaticClass()) ? (ABuildingActor*)InActor : nullptr;

        UE_LOG(LogVelora, Log, TEXT("Opened channel for %s to %s at %.3f, %.1f m away, cull distance %.1f m"), *InActor->GetName(), PlayerController ? *PlayerController->GetName() : TEXT("None"), InActor->GetWorld()->GetTimeSeconds(), ViewTarget ? (InActor->GetActorLocation() - ViewTarget->GetActorLocation()).Size() / 100.f : -1.f, BuildingActor ? BuildingActor->GetCullDistance() / 100.f : -1.f);
    }
}

void UActorChannel::SetChannelActorForDestroy(FActorDestructionInfo* DestructInfo)
{
    if (!GetClosing() && (GetConnection()->GetState() == USOCK_Open || GetConnection()->GetState() == USOCK_Pending))
    {
        auto CloseBunch = new FOutBunch(this, 1);
        CloseBunch->SetReliable(1);
        CloseBunch->SetDormant(0);

        GetConnection()->GetPackageMap()->WriteObject(*(FArchive*)CloseBunch, DestructInfo->ObjOuter.Get(), DestructInfo->NetGUID, DestructInfo->PathName);

        SendBunch(CloseBunch, 0);

        delete CloseBunch;
    }
}
