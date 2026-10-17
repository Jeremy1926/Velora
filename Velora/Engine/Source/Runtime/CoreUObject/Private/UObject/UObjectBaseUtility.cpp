#include "pch.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "hde/hde64.h"

bool UObjectBaseUtility::IsA(const UClass* SomeBase) const
{
    return GetClass()->IsChildOf(SomeBase);
}

void* UObjectBaseUtility::GetInterfaceAddress(UClass* InterfaceClass)
{
    static auto GetInterfaceAddress = []
    {
        const uint8_t* GetAbilitySystemComponent = nullptr;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xC3)
                break;

            if (Instruction.opcode == 0xE8)
                GetAbilitySystemComponent = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        const uint8_t* GetAbilitySystemComponentFromActor = nullptr;

        for (auto Address = GetAbilitySystemComponent; !GetAbilitySystemComponentFromActor;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 || Instruction.opcode == 0xE9)
                GetAbilitySystemComponentFromActor = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        auto Address = GetAbilitySystemComponentFromActor;

        for (auto Calls = 0;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 2)
                return (void* (*)(UObjectBaseUtility*, UClass*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    return GetInterfaceAddress(this, InterfaceClass);
}
