#include "pch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FMemory::Free(void* Original)
{
    static auto Free = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")));
        auto End = Scanner::FindFunctionEnd(Function);
        const uint8_t* Instructions[2] = {};

        for (auto Address = Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && Instructions[0] && !memcmp(Instructions[0], "\x48\x85\xC9", 3) && !memcmp(Instructions[1], "\x74\x05", 2))
                return (void (*)(void*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Instructions[0] = Instructions[1];
            Instructions[1] = Address;
            Address += Length;
        }

        return (void (*)(void*))nullptr;
    }();

    Free(Original);
}
