#include "pch.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto SendRequestNow = []
{
    auto Reference = Scanner::FindStringReference(TEXT("FATAL: Should not be possible to issue dedicated server commands without WITH_SERVER_CODE"));
    auto Function = Scanner::FindFunction(Reference);
    const uint8_t* Client = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0x84)
        {
            auto Target = Address + Length + (int32_t)Instruction.imm.imm32;

            hde64s TargetInstruction;
            auto TargetLength = hde64_disasm(Target, &TargetInstruction);

            if (Target <= Reference && Reference < Target + TargetLength)
                return PatchBranch(Address, Client);

            Client = Target;
        }

        Address += Length;
    }

    return (const uint8_t*)nullptr;
}();
