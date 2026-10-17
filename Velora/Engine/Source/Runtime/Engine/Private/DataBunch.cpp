#include "pch.h"
#include "Net/DataBunch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"
#include "hde/hde64.h"

struct FCloseBunch
{
    const uint8_t* Constructor;
    int32_t Flags;
    uint8_t DormantMask;
    uint8_t ReliableMask;
};

static auto CloseBunch = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")));
    auto End = Scanner::FindFunctionEnd(Function);
    FCloseBunch Bunch = {};
    int32_t Frame = 0;
    bool bClose = false;
    uint8_t Reliable = 0;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End && !Bunch.ReliableMask;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);
        auto Disp = Instruction.modrm_mod == 1 ? (int32_t)(int8_t)Instruction.disp.disp8 : (int32_t)Instruction.disp.disp32;

        if (!Bunch.Constructor && Instruction.opcode == 0x8D && !Instruction.rex_r && Instruction.modrm_reg == 1 && Instruction.modrm_rm == 4)
            Frame = Disp;
        else if (Instruction.opcode == 0xB0 && Instruction.rex_b && Instruction.imm.imm8 == 1)
            bClose = true;
        else if (bClose && !Bunch.Constructor && Instruction.opcode == 0xE8)
            Bunch.Constructor = Address + Length + (int32_t)Instruction.imm.imm32;
        else if (Bunch.Constructor && !Bunch.Flags && Instruction.opcode == 0x0F && Instruction.opcode2 == 0xB6 && Instruction.modrm_rm == 4)
            Bunch.Flags = Disp - Frame;
        else if (Bunch.Flags && !Bunch.DormantMask && Instruction.opcode == 0x24)
            Bunch.DormantMask = (uint8_t)~Instruction.imm.imm8;
        else if (Bunch.Flags && !Reliable && Instruction.opcode == 0x80 && Instruction.modrm == 0xC9)
            Reliable = Instruction.imm.imm8;
        else if (Reliable && Instruction.opcode == 0xC0 && Instruction.modrm == 0xE1)
            Bunch.ReliableMask = (uint8_t)(Reliable << Instruction.imm.imm8);

        Address += Length;
    }

    return Bunch;
}();

void* FOutBunch::operator new(size_t Size)
{
    static auto New = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s")));
        auto End = Scanner::FindFunctionEnd(Function);
        std::pair<uint32_t, const uint8_t*> Allocation = {};
        uint32_t BunchSize = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xB9 && !Instruction.rex)
                BunchSize = Instruction.imm.imm32;
            else if (Instruction.opcode == 0xE8)
            {
                auto Call = Address + Length + (int32_t)Instruction.imm.imm32;

                if (Call == CloseBunch.Constructor)
                    return Allocation;

                Allocation = { BunchSize, Call };
            }

            Address += Length;
        }

        return Allocation;
    }();

    return ((void* (*)(size_t))New.second)(New.first);
}

void FOutBunch::operator delete(void* Ptr)
{
    FMemory::Free(Ptr);
}

FOutBunch::FOutBunch(UChannel* InChannel, bool bInClose)
{
    ((FOutBunch* (*)(FOutBunch*, UChannel*, bool))CloseBunch.Constructor)(this, InChannel, bInClose);
}

FOutBunch::~FOutBunch()
{
    (**(void (***)(FOutBunch*, uint32))this)(this, 0);
}

void FOutBunch::SetReliable(bool bInReliable)
{
    auto& Flags = *((uint8_t*)this + CloseBunch.Flags);

    Flags = (Flags & ~CloseBunch.ReliableMask) | (bInReliable ? CloseBunch.ReliableMask : 0);
}

void FOutBunch::SetDormant(bool bInDormant)
{
    auto& Flags = *((uint8_t*)this + CloseBunch.Flags);

    Flags = (Flags & ~CloseBunch.DormantMask) | (bInDormant ? CloseBunch.DormantMask : 0);
}
