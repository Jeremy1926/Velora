#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto CommandletFlags = []
{
    auto Reference = Scanner::FindReference(Scanner::FindString(TEXT("Cannot run editor commandlet %s with game executable.")));
    DWORD64 ImageBase = 0;
    auto Function = RtlLookupFunctionEntry((DWORD64)Reference, &ImageBase, nullptr);
    std::array<bool*, 2> Flags = {};

    for (auto Address = (const uint8_t*)ImageBase + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x88 && Instruction.modrm == 0x05)
        {
            Flags[0] = Flags[1];
            Flags[1] = (bool*)(Address + Length + (int32_t)Instruction.disp.disp32);
        }

        Address += Length;
    }

    return Flags;
}();

bool& GIsClient = *CommandletFlags[0];
bool& GIsServer = *CommandletFlags[1];
