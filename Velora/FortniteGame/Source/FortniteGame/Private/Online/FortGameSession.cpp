#include "pch.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto ValidatePlayer = []
{
    auto Reference = Scanner::FindStringReference(TEXT("No reservation in game."));
    auto Function = Scanner::FindFunction(Reference);
    auto End = Scanner::FindFunctionEnd(Function);
    std::array<const uint8_t*, 2> Branches = {};

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x75)
            Branches[0] = Address;

        Address += Length;
    }

    for (auto Address = Branches[0] + 2 + (int8_t)Branches[0][1]; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && !memcmp(Address + Length, "\x84\xC0\x75", 3))
        {
            Branches[1] = Address + Length + 2;
            break;
        }

        Address += Length;
    }

    for (auto Branch : Branches)
        Patch(Branch, 0xEB);

    return Branches;
}();
