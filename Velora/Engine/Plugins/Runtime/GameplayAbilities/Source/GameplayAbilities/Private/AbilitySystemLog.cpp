#include "pch.h"
#include "AbilitySystemLog.h"
#include "Scanner.h"
#include "hde/hde64.h"

FLogCategoryBase& LogAbilitySystem = *[]
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!")));

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x80 && Instruction.modrm_reg == 7 && (Instruction.modrm & 0xC7) == 0x05)
            return (FLogCategoryBase*)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }
}();
