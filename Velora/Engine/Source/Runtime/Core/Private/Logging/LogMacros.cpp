#include "pch.h"
#include "Logging/LogMacros.h"
#include "Scanner.h"
#include "hde/hde64.h"

decltype(FMsg::Logf_Internal) FMsg::Logf_Internal = []
{
    auto Reference = Scanner::FindStringReference(TEXT("LoadMap: failed to Listen(%s)"));
    auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

    for (auto Address = Reference; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            return (decltype(FMsg::Logf_Internal))(Address + Length + (int32_t)Instruction.imm.imm32);

        Address += Length;
    }

    return (decltype(FMsg::Logf_Internal))nullptr;
}();
