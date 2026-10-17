#include "pch.h"
#include "FortGameInstance.h"
#include "Scanner.h"
#include "hde/hde64.h"

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("TeamChatroom")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                hde64s Assignment;
                hde64_disasm(Address + Length, &Assignment);

                if (Assignment.opcode == 0x81 && Assignment.rex_w && Assignment.modrm_mod == 3 && Assignment.modrm_reg == 0)
                    return (int32_t)Assignment.imm.imm32;
            }

            Address += Length;
        }

        return 0;
    }();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}
