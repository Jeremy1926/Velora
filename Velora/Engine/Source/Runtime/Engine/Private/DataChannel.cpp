#include "pch.h"
#include "Engine/ActorChannel.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "Net/DataBunch.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "hde/hde64.h"

UNetConnection* UChannel::GetConnection() const
{
    static auto Connection = GetClass()->FindPropertyByName(FName(TEXT("Connection")));

    return *Connection->ContainerPtrToValuePtr<UNetConnection*>(this);
}

bool UChannel::GetClosing() const
{
    static auto Closing = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xF6 && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 1)
                return std::pair((int32_t)(int8_t)Instruction.disp.disp8, Instruction.imm.imm8);

            Address += Length;
        }

        return std::pair<int32_t, uint8_t>();
    }();

    return *((uint8_t*)this + Closing.first) & Closing.second;
}

bool UChannel::GetDormant() const
{
    static auto Dormant = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xF6 && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 1)
                return std::pair((int32_t)(int8_t)Instruction.disp.disp8, Instruction.imm.imm8);

            Address += Length;
        }

        return std::pair<int32_t, uint8_t>();
    }();

    return *((uint8_t*)this + Dormant.first) & Dormant.second;
}

bool UChannel::GetPendingDormancy() const
{
    static auto bPendingDormancy = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("StartBecomingDormant: %s")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x81 && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 1)
                return std::pair((int32_t)(int8_t)Instruction.disp.disp8, Instruction.imm.imm32);

            Address += Length;
        }

        return std::pair<int32_t, uint32_t>();
    }();

    return *(uint32_t*)((uint8_t*)this + bPendingDormancy.first) & bPendingDormancy.second;
}

int32 UChannel::GetNumOutRec() const
{
    static auto NumOutRec = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x83 && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 1 && Instruction.imm.imm8 == 0)
                return (int32_t)(int8_t)Instruction.disp.disp8;

            Address += Length;
        }

        return 0;
    }();

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
    static auto SendBunch = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")));
        auto End = Scanner::FindFunctionEnd(Function);
        bool bClose = false;
        bool bCloseBunch = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xB0 && Instruction.rex_b && Instruction.imm.imm8 == 1)
                bClose = true;
            else if (bClose && Instruction.opcode == 0xE8)
                bCloseBunch = true;
            else if (bCloseBunch && Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;
    FPacketIdRange Result;

    return *(*(FPacketIdRange* (**)(UChannel*, FPacketIdRange*, FOutBunch*, bool))(VTable + SendBunch))(this, &Result, Bunch, Merge);
}

AActor* UActorChannel::GetActor() const
{
    static auto Actor = GetClass()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}

static auto LastUpdateTime = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")));
    auto End = Scanner::FindFunctionEnd(Function);
    bool bTime = false;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0x5A && !Instruction.p_66 && !Instruction.p_rep)
            bTime = true;
        else if (bTime && Instruction.opcode == 0x0F && Instruction.opcode2 == 0x11 && Instruction.p_rep == 0xF2 && Instruction.modrm_mod == 2)
            return (int32_t)Instruction.disp.disp32;

        Address += Length;
    }

    return 0;
}();

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
    static auto Close = (void (*)(UActorChannel*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")))->BeginAddress);

    Close(this);
}

void UActorChannel::StartBecomingDormant()
{
    static auto StartBecomingDormant = (void (*)(UActorChannel*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("StartBecomingDormant: %s")))->BeginAddress);

    StartBecomingDormant(this);
}

bool UActorChannel::ReplicateActor()
{
    static auto ReplicateActor = (bool (*)(UActorChannel*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d")))->BeginAddress);

    return ReplicateActor(this);
}

void UActorChannel::SetChannelActor(AActor* InActor)
{
    static auto SetChannelActor = (void (*)(UActorChannel*, AActor*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("SetChannelActor: ChIndex: %i, Actor: %s, NetGUID: %s")))->BeginAddress);

    SetChannelActor(this, InActor);
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
