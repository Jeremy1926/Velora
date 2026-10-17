#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FWeakObjectPtr::operator=(const UObject* Object)
{
    static auto Assign = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")));
        auto End = Scanner::FindFunctionEnd(Function);
        const uint8_t* Call = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;
            else if (Instruction.opcode == 0x81 && Instruction.rex_w && Instruction.modrm == 0xC1)
                break;

            Address += Length;
        }

        return (void (*)(FWeakObjectPtr*, const UObject*))Call;
    }();

    Assign(this, Object);
}

UObject* FWeakObjectPtr::Get() const
{
    return Internal_Get(false);
}

bool FWeakObjectPtr::IsValid() const
{
    return Internal_IsValid(false, false);
}
