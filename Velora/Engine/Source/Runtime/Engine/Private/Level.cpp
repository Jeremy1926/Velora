#include "pch.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "hde/hde64.h"

UWorld* ULevel::GetOwningWorld() const
{
    static auto OwningWorld = GetClass()->FindPropertyByName(FName(TEXT("OwningWorld")));

    return *OwningWorld->ContainerPtrToValuePtr<UWorld*>(this);
}

bool ULevel::IsAssociatingLevel() const
{
    static auto bIsAssociatingLevel = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("sorting actor list")));
        auto End = Scanner::FindFunctionEnd(Function);
        int32_t Offset = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (!Offset && Instruction.opcode == 0x8D && Instruction.rex_w && Instruction.modrm_mod == 2 && Instruction.modrm_rm == 2)
                Offset = (int32_t)Instruction.disp.disp32;
            else if (Offset && Instruction.opcode == 0x80 && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 0)
                return std::pair(Offset, Instruction.imm.imm8);

            Address += Length;
        }

        return std::pair<int32_t, uint8_t>();
    }();

    return *((uint8_t*)this + bIsAssociatingLevel.first) & bIsAssociatingLevel.second;
}

bool ULevel::HasVisibilityChangeRequestPending() const
{
    return (GetOwningWorld() && (this == GetOwningWorld()->GetCurrentLevelPendingVisibility() || this == GetOwningWorld()->GetCurrentLevelPendingInvisibility()));
}
