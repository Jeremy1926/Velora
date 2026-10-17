#include "pch.h"
#include "Misc/CommandLine.h"
#include "Scanner.h"
#include "hde/hde64.h"

const TCHAR* FCommandLine::Get()
{
    static auto Get = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("NOCONSOLE"));
        auto Function = Scanner::FindFunction(Reference);
        const uint8_t* Call = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (const TCHAR* (*)())Call;
    }();

    return Get();
}
