#include "pch.h"
#include "GameplayTagsManager.h"
#include "Scanner.h"
#include "hde/hde64.h"

UGameplayTagsManager& UGameplayTagsManager::Get()
{
    static auto SingletonManager = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Gameplay.Effect.InstantDeath.Silent")));

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            hde64s NextInstruction;
            hde64_disasm(Address + Length, &NextInstruction);

            if (Instruction.opcode == 0x8B && Instruction.rex_w && (Instruction.modrm & 0xC7) == 0x05 && NextInstruction.opcode == 0x85 && NextInstruction.modrm_mod == 3 && NextInstruction.modrm_reg == Instruction.modrm_reg && NextInstruction.modrm_rm == Instruction.modrm_reg)
                return (UGameplayTagsManager**)(Address + Length + (int32_t)Instruction.disp.disp32);

            Address += Length;
        }
    }();

    if (*SingletonManager == nullptr)
    {
        InitializeManager();
    }

    return **SingletonManager;
}

void UGameplayTagsManager::InitializeManager()
{
    static auto InitializeManager = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Gameplay.Effect.InstantDeath.Silent")));
        auto bSingletonManager = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bSingletonManager && Instruction.opcode == 0xE8)
                return (void (*)())(Address + Length + (int32_t)Instruction.imm.imm32);

            hde64s NextInstruction;
            hde64_disasm(Address + Length, &NextInstruction);

            bSingletonManager |= Instruction.opcode == 0x8B && Instruction.rex_w && (Instruction.modrm & 0xC7) == 0x05 && NextInstruction.opcode == 0x85 && NextInstruction.modrm_mod == 3 && NextInstruction.modrm_reg == Instruction.modrm_reg && NextInstruction.modrm_rm == Instruction.modrm_reg;
            Address += Length;
        }
    }();

    InitializeManager();
}

FGameplayTag UGameplayTagsManager::RequestGameplayTag(FName TagName, bool ErrorIfNotFound) const
{
    static auto RequestGameplayTag = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Gameplay.Effect.InstantDeath.Silent"));
        auto Calls = 0;

        for (auto Address = Reference;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 2)
                return (FGameplayTag* (*)(const UGameplayTagsManager*, FGameplayTag*, FName, bool))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    FGameplayTag Result;

    return *RequestGameplayTag(this, &Result, TagName, ErrorIfNotFound);
}
