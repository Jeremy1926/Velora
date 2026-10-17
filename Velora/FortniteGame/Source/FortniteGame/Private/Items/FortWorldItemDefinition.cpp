#include "pch.h"
#include "Items/FortWorldItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* UFortWorldItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldItemDefinition"));

    return Class;
}

bool UFortWorldItemDefinition::ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const
{
    static auto ServerExecute = []
    {
        const uint8_t* EquipWeaponDefinition = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition")))->GetNativeFunc()); !EquipWeaponDefinition;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                EquipWeaponDefinition = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeaponItemDefinition"));

        for (auto Index = 0;; Index++)
        {
            auto Function = Scanner::FindFunction(VTable[Index]);

            if (!Function)
                continue;

            auto End = Scanner::FindFunctionEnd(Function);

            for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
            {
                hde64s Instruction;
                auto Length = hde64_disasm(Address, &Instruction);

                if (Instruction.opcode == 0xE8 && Address + Length + (int32_t)Instruction.imm.imm32 == EquipWeaponDefinition)
                    return Index * (int32_t)sizeof(void*);

                Address += Length;
            }
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UFortWorldItemDefinition*, UFortWorldItem*, AFortPlayerController*))(VTable + ServerExecute))(this, Item, PlayerController);
}
