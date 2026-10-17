#include "pch.h"
#include "Misc/Guid.h"
#include "Scanner.h"
#include "hde/hde64.h"

FGuid FGuid::NewGuid()
{
    static auto NewGuid = []
    {
        auto Address = Scanner::FindStringReference(TEXT("FIX GUID FOR: %s"));

        for (auto Calls = 0;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 3)
                return (FGuid* (*)(FGuid*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    FGuid Result;
    NewGuid(&Result);

    return Result;
}
