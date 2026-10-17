#include "pch.h"
#include "Containers/UnrealString.h"
#include "Scanner.h"
#include "hde/hde64.h"

template<>
void TArray<TCHAR>::ResizeGrow(int32 OldNum)
{
    static auto ResizeGrow = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")));
        auto End = Scanner::FindFunctionEnd(Function);
        bool bOldNumZero = false;

        for (auto Address = Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bOldNumZero)
                    return (void (*)(TArray<TCHAR>*, int32))(Address + Length + (int32_t)Instruction.imm.imm32);

                bOldNumZero = false;
            }
            else if (!memcmp(Address, "\x33\xD2", 2))
                bOldNumZero = true;

            Address += Length;
        }

        return (void (*)(TArray<TCHAR>*, int32))nullptr;
    }();

    ResizeGrow(this, OldNum);
}
