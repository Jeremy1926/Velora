#include "pch.h"
#include "Engine/ActorChannel.h"
#include "Engine/ControlChannel.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "Misc/EngineVersion.h"
#include "Net/DataBunch.h"
#include "UObject/NameTypes.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UNetConnection* UChannel::GetConnection() const
{
    static auto Connection = GetClass()->FindPropertyByName(FName(TEXT("Connection")));

    return *Connection->ContainerPtrToValuePtr<UNetConnection*>(this);
}

int32 UChannel::GetChIndex() const
{
    static auto ChIndex = Anchor::Offset(TEXT("UChannel::ChIndex"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov eax, [rbx+0x*]; mov [rsi+0x*], eax")
        .Disp();

    return *(const int32*)((const uint8_t*)this + ChIndex);
}

FName UChannel::GetChName() const
{
    static auto ChName = Anchor::Offset(TEXT("UChannel::ChName"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov eax, [rbx+0x*]; mov [rsi+0x*], eax")
        .Find("mov *, [rbx+0x*]")
        .Disp();

    return TGameLayout<FName>::Load((const uint8_t*)this + ChName);
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

FScopedActorRoleSwap::FScopedActorRoleSwap(AActor* InActor)
    : Actor(InActor && InActor->GetRemoteRole() == ROLE_Authority ? InActor : nullptr)
{
    if (Actor)
        Actor->SwapRoles();
}

FScopedActorRoleSwap::~FScopedActorRoleSwap()
{
    if (Actor)
        Actor->SwapRoles();
}

int64 UChannel::Close(EChannelCloseReason Reason)
{
    static auto Close = Anchor::VirtualFunction<int64 (*)(UChannel*, EChannelCloseReason)>(TEXT("UChannel::Close"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .SlotIn(TEXT("/Script/Engine.Default__Channel"))
        .Get();

    return Close(this, Reason);
}

UClass* UControlChannel::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.ControlChannel"));

    return Class;
}

int64 UControlChannel::SendDestructionInfo(FActorDestructionInfo* DestructionInfo)
{
    auto Connection = GetConnection();
    const auto State = Connection->GetState();

    if (GetClosing() || (State != USOCK_Open && State != USOCK_Pending))
        return 0;

    auto ObjOuter = DestructionInfo->GetObjOuter().Get();

    if (!ObjOuter)
        return 0;

    auto PackageMap = Connection->GetPackageMap();
    auto InfoBunch = new FOutBunch();
    InfoBunch->SetPackageMap(PackageMap);
    InfoBunch->SetReliable(true);

    uint8 MessageType = 22;
    InfoBunch->Serialize(&MessageType, 1);

    uint8 Reason = (uint8)DestructionInfo->GetReason();
    InfoBunch->Serialize(&Reason, 1);

    PackageMap->WriteObject(*(FArchive*)InfoBunch, ObjOuter, DestructionInfo->GetNetGUID(), DestructionInfo->GetPathName());
    SendBunch(InfoBunch, false);

    const int64 NumBits = InfoBunch->GetNumBits();
    delete InfoBunch;

    return NumBits;
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

bool UActorChannel::GetSpawnAcked() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *((uint8_t*)Actor->ContainerPtrToValuePtr<AActor*>(this) + sizeof(AActor*) + sizeof(FNetworkGUID) + sizeof(float) + 2 * sizeof(double)) & 1;
}

bool& UActorChannel::GetHoldQueuedExportBunchesAndGUIDs()
{
    static auto bHoldQueuedExportBunchesAndGUIDs = Anchor::Offset(TEXT("UActorChannel::bHoldQueuedExportBunchesAndGUIDs"))
        .VirtualMatch(TEXT("/Script/Engine.Default__ActorChannel"), "mov [rsp+0x8], rbx; mov [rsp+0x10], rsi; push rdi; sub rsp, 0x20; cmp byte ptr [rcx+0x*], 0x0; mov rsi, rdx; mov rbx, rcx; jnz 0x*; mov rcx, [rcx+0x*]; mov rcx, [rcx+0x*]; call 0x*")
        .Find("cmp byte ptr [rcx+0x*], 0x0")
        .Disp();

    return *(bool*)((uint8_t*)this + bHoldQueuedExportBunchesAndGUIDs);
}

int64 UActorChannel::ReplicateActor()
{
    static auto ReplicateActor = Anchor::Function<int64 (*)(UActorChannel*)>(TEXT("UActorChannel::ReplicateActor"))
        .String(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"))
        .Start()
        .Get();

    const auto Result = ReplicateActor(this);

    return FEngineVersion::Current().GetMajor() >= 5 ? Result : (uint8)Result;
}

void UActorChannel::SetChannelActor(AActor* InActor, ESetChannelActorFlags Flags)
{
    static auto SetChannelActor = Anchor::Function<void (*)(UActorChannel*, AActor*, ESetChannelActorFlags)>(TEXT("UActorChannel::SetChannelActor"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .FindCallToString(TEXT("Created channel %i of type %i"))
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("40 55 53 56 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 45 33 E4 48 8D 3D ? ? ? ? 44 89 A5 ? ? ? ? 48 8B D9 48 8B 41 28 45 8B F8 48 8B F2 45 8B F4 48 8B 48 58 48 85 C9 74 ? 4C 39 A1 ? ? ? ? 74 ? 48 85 D2 74")
        .Get();

    SetChannelActor(this, InActor, Flags);
}

int64 UActorChannel::SetChannelActorForDestroy(FActorDestructionInfo* DestructInfo)
{
    if (FEngineVersion::Current().GetMajor() >= 5)
    {
        static auto SetChannelActorForDestroy = Anchor::Function<int64 (*)(UActorChannel*, FActorDestructionInfo*)>(TEXT("UActorChannel::SetChannelActorForDestroy"))
            .Pattern("40 55 53 56 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 F6 4C 8B F2 F6 41 30 02 4C 8B F9 0F 85 ? ? ? ? 48 8B 41 28 44 8B 80 ? ? ? ? 41 83 E8 02 41 83 F8 01 0F 87")
            .Get();

        return SetChannelActorForDestroy(this, DestructInfo);
    }

    if (!GetClosing() && (GetConnection()->GetState() == USOCK_Open || GetConnection()->GetState() == USOCK_Pending))
    {
        auto CloseBunch = new FOutBunch(this, 1);
        CloseBunch->SetReliable(1);
        CloseBunch->SetDormant(0);

        GetConnection()->GetPackageMap()->WriteObject(*(FArchive*)CloseBunch, DestructInfo->GetObjOuter().Get(), DestructInfo->GetNetGUID(), DestructInfo->GetPathName());

        SendBunch(CloseBunch, 0);

        delete CloseBunch;
    }

    return 0;
}

void UActorChannel::WriteFieldHeaderAndPayload(FNetBitWriter& Bunch, const FClassNetCache* ClassCache, const FFieldNetCache* FieldCache, FNetFieldExportGroup* NetFieldExportGroup, FNetBitWriter& Payload, bool bIgnoreInternalAck)
{
    static auto WriteFieldHeaderAndPayload = Anchor::Function<void (*)(UActorChannel*, FNetBitWriter&, const FClassNetCache*, const FFieldNetCache*, FNetFieldExportGroup*, FNetBitWriter&, bool)>(TEXT("UActorChannel::WriteFieldHeaderAndPayload"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("and dword ptr [rsp+0x*], 0x0")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*; lea rax, [rbp+0x*]; xor r9d, r9d; lea r8, [rbp-0x*]")
        .Target()
        .Get();

    WriteFieldHeaderAndPayload(this, Bunch, ClassCache, FieldCache, NetFieldExportGroup, Payload, bIgnoreInternalAck);
}

int32 UActorChannel::WriteContentBlockPayload(UObject* Obj, FNetBitWriter& Bunch, const bool bHasRepLayout, FNetBitWriter& Payload)
{
    static auto WriteContentBlockPayload = Anchor::Function<int32 (*)(UActorChannel*, UObject*, FNetBitWriter&, bool, FNetBitWriter&)>(TEXT("UActorChannel::WriteContentBlockPayload"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("and dword ptr [rsp+0x*], 0x0")
        .Find("call 0x*")
        .Target()
        .Find("xor r9d, r9d; lea r8, [rbp-0x*]; mov [rsp+0x20], rax; mov rdx, r*; mov rcx, r*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return WriteContentBlockPayload(this, Obj, Bunch, bHasRepLayout, Payload);
}
