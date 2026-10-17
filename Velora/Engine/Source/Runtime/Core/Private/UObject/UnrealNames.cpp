#include "pch.h"
#include "UObject/NameTypes.h"
#include "Scanner.h"
#include "hde/hde64.h"

FName::FName(const WIDECHAR* Name, EFindName FindType)
{
    static auto Constructor = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")));
        auto End = Scanner::FindFunctionEnd(Function);
        bool bFindTypeAdd = false;

        for (auto Address = Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bFindTypeAdd)
                    return (void (*)(FName*, const WIDECHAR*, EFindName))(Address + Length + (int32_t)Instruction.imm.imm32);

                bFindTypeAdd = false;
            }
            else if (Instruction.opcode == 0xB8 && Instruction.rex_b && Instruction.imm.imm32 == FNAME_Add)
                bFindTypeAdd = true;

            Address += Length;
        }

        return (void (*)(FName*, const WIDECHAR*, EFindName))nullptr;
    }();

    Constructor(this, Name, FindType);
}
