#include "pch.h"
#include "Player/FortPlayerState.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* AFortPlayerState::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerState"));

    return Class;
}

void AFortPlayerState::ApplyCharacterCustomization(APawn* Pawn)
{
    static auto ApplyCharacterCustomization = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerState"));
        int32_t ServerSetShowHeroBackpack_Implementation = 0;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerState:ServerSetShowHeroBackpack")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xC3)
                break;

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                ServerSetShowHeroBackpack_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        int32_t ApplyHeroPartDisplaySettings = 0;

        for (auto Address = VTable[ServerSetShowHeroBackpack_Implementation / sizeof(void*)]; !ApplyHeroPartDisplaySettings;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 4 && Instruction.modrm_mod == 2)
                ApplyHeroPartDisplaySettings = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto Function = Scanner::FindFunction(VTable[ApplyHeroPartDisplaySettings / sizeof(void*)]);
        auto End = Scanner::FindFunctionEnd(Function);
        const uint8_t* ApplyCustomizationToCharacter = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                ApplyCustomizationToCharacter = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        Function = Scanner::FindFunction(ApplyCustomizationToCharacter);
        End = Scanner::FindFunctionEnd(Function);
        auto Begin = (const uint8_t*)Scanner::Base() + Function->BeginAddress;

        for (auto Address = Begin; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 || Instruction.opcode == 0xE9)
            {
                auto Target = Address + Length + (int32_t)Instruction.imm.imm32;

                if (Target < Begin || Target >= End)
                    return (void (*)(AFortPlayerState*, APawn*))Target;
            }

            Address += Length;
        }

        return (void (*)(AFortPlayerState*, APawn*))nullptr;
    }();

    ApplyCharacterCustomization(this, Pawn);
}
