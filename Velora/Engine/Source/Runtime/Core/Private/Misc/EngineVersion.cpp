#include "pch.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"
#include "hde/hde64.h"

const FEngineVersion& FEngineVersion::Current()
{
    static auto Current = []
    {
        auto Reference = Scanner::FindReference(Scanner::FindString(TEXT("Engine Version: %s")));
        DWORD64 ImageBase = 0;
        auto Function = RtlLookupFunctionEntry((DWORD64)Reference, &ImageBase, nullptr);
        const uint8* Calls[2] = {};

        for (auto Address = (const uint8*)ImageBase + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            Address += hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                Calls[0] = Calls[1];
                Calls[1] = Address + (int32)Instruction.imm.imm32;
            }
        }

        return (const FEngineVersion& (*)())Calls[0];
    }();

    return Current();
}
