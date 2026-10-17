#include "pch.h"
#include "Math/Rotator.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

FVector FRotator::Vector() const
{
    static auto Vector = []
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));
        const uint8_t* Call = nullptr;

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (FVector* (*)(const FRotator*, FVector*))Call;
    }();

    FVector Result;

    return *Vector(this, &Result);
}
