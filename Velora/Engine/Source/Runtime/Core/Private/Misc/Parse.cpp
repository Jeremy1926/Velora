#include "pch.h"
#include "Misc/Parse.h"
#include "Scanner.h"
#include "hde/hde64.h"

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
    static auto ParseParam = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("NOCONSOLE"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                return (bool (*)(const TCHAR*, const TCHAR*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }

        return (bool (*)(const TCHAR*, const TCHAR*))nullptr;
    }();

    return ParseParam(Stream, Param);
}
