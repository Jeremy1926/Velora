#include "pch.h"
#include "Math/Rotator.h"
#include "Math/TransformVectorized.h"
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

const FTransform FTransform::Identity;

FVector FMath::VRand()
{
    static auto VRand = []
    {
        const uint8_t* RandomUnitVector = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVector")))->GetNativeFunc()); !RandomUnitVector;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                RandomUnitVector = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        for (auto Address = RandomUnitVector;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                return (FVector* (*)(FVector*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    FVector Result;

    return *VRand(&Result);
}

FVector FMath::VRandCone(FVector const& Dir, float ConeHalfAngleRad)
{
    static auto VRandCone = []
    {
        const uint8_t* RandomUnitVectorInCone = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInCone")))->GetNativeFunc()); !RandomUnitVectorInCone;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                RandomUnitVectorInCone = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        return (FVector* (*)(FVector*, FVector const&, float))RandomUnitVectorInCone;
    }();

    FVector Result;

    return *VRandCone(&Result, Dir, ConeHalfAngleRad);
}
