#include "pch.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "hde/hde64.h"

UClass* UFortAbilitySet::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySet"));

    return Class;
}

FFortAbilitySetHandle UFortAbilitySet::EquipAbilitySet(IAbilitySystemInterface* AbilitySystemInterface, UObject* OverrideSourceObject, IAbilitySystemInterface* SourceAbilitySystemInterface)
{
    static auto EquipAbilitySet = []
    {
        auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:EquipFortAbilitySet")))->GetNativeFunc());

        for (auto bFinished = false; !bFinished;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            bFinished = Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        const uint8_t* EquipFortAbilitySet = nullptr;

        for (auto Calls = 0; !EquipFortAbilitySet;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 2)
                EquipFortAbilitySet = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        for (auto Calls = 0;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(EquipFortAbilitySet, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 2)
                return (FFortAbilitySetHandle* (*)(UFortAbilitySet*, FFortAbilitySetHandle*, IAbilitySystemInterface*, UObject*, IAbilitySystemInterface*))(EquipFortAbilitySet + Length + (int32_t)Instruction.imm.imm32);

            EquipFortAbilitySet += Length;
        }
    }();

    FFortAbilitySetHandle Handle;

    EquipAbilitySet(this, &Handle, AbilitySystemInterface, OverrideSourceObject, SourceAbilitySystemInterface);

    return Handle;
}
