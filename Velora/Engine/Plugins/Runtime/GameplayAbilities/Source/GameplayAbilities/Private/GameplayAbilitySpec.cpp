#include "pch.h"
#include "GameplayAbilitySpec.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FGameplayAbilitySpecHandle::GenerateNewHandle()
{
    static auto& GHandle = *[]
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("AddActiveGameplayEffectGrantedTagsAndModifiers called with null Def!")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                auto Callee = Scanner::FindFunction(Address + Length + (int32_t)Instruction.imm.imm32);
                auto CalleeEnd = Callee ? Scanner::FindFunctionEnd(Callee) : nullptr;

                for (auto CalleeAddress = (const uint8_t*)Scanner::Base() + (Callee ? Callee->BeginAddress : 0); CalleeAddress < CalleeEnd;)
                {
                    hde64s CalleeInstruction;
                    auto CalleeLength = hde64_disasm(CalleeAddress, &CalleeInstruction);

                    if (CalleeInstruction.opcode == 0xFF && !CalleeInstruction.rex_w && CalleeInstruction.modrm == 0x05)
                        return (int32*)(CalleeAddress + CalleeLength + (int32_t)CalleeInstruction.disp.disp32);

                    CalleeAddress += CalleeLength;
                }
            }

            Address += Length;
        }

        return (int32*)nullptr;
    }();

    Handle = GHandle++;
}
