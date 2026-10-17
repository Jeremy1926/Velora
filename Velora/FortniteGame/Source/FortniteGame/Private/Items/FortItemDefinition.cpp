#include "pch.h"
#include "Items/FortItemDefinition.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

int32 UFortItemDefinition::GetMinLevel() const
{
    static auto MinLevel = GetClass()->FindPropertyByName(FName(TEXT("MinLevel")));

    return *MinLevel->ContainerPtrToValuePtr<int32>(this);
}

EFortItemType UFortItemDefinition::GetItemType() const
{
    static auto ItemType = GetClass()->FindPropertyByName(FName(TEXT("ItemType")));

    return *ItemType->ContainerPtrToValuePtr<EFortItemType>(this);
}

EFortRarity UFortItemDefinition::GetRarity() const
{
    static auto GetRarity = FindFunctionChecked(FName(TEXT("GetRarity")));

    struct
    {
        EFortRarity ReturnValue;
    } Parms;

    const_cast<UFortItemDefinition*>(this)->ProcessEvent(GetRarity, &Parms);

    return Parms.ReturnValue;
}

int32 UFortItemDefinition::ClampItemLevel(int32 ItemLevel) const
{
    static auto ClampItemLevel = []
    {
        const uint8_t* CreateTemporaryItemInstanceBP = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition:CreateTemporaryItemInstanceBP")))->GetNativeFunc()); !CreateTemporaryItemInstanceBP;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                CreateTemporaryItemInstanceBP = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        for (auto Address = CreateTemporaryItemInstanceBP;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const UFortItemDefinition*, int32))(VTable + ClampItemLevel))(this, ItemLevel);
}

bool UFortItemDefinition::DoesAllowMultipleStacks() const
{
    static auto DoesAllowMultipleStacks = []
    {
        auto bAllowMultipleStacks = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("bAllowMultipleStacks")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortItemDefinition"));

        for (auto Index = 0;; Index++)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(VTable[Index], &Instruction);

            hde64s NextInstruction;
            hde64_disasm(VTable[Index] + Length, &NextInstruction);

            if (Instruction.opcode == 0x8B && Instruction.modrm_mod == 2 && Instruction.modrm_rm == 1 && (int32_t)Instruction.disp.disp32 == bAllowMultipleStacks && NextInstruction.opcode == 0x83 && NextInstruction.modrm_reg == 4)
                return Index * (int32_t)sizeof(void*);
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UFortItemDefinition*))(VTable + DoesAllowMultipleStacks))(this);
}

int32 UFortItemDefinition::GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent) const
{
    static auto GetMaxStackSize = []
    {
        auto MaxStackSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("MaxStackSize")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortItemDefinition"));

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

                if (Instruction.opcode == 0x81 && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 2 && (int32_t)Instruction.disp.disp32 == MaxStackSize && Instruction.imm.imm32 == 0x7FFFFFFE)
                    return Index * (int32_t)sizeof(void*);

                Address += Length;
            }
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const UFortItemDefinition*, const UAbilitySystemComponent*))(VTable + GetMaxStackSize))(this, AbilitySystemComponent);
}
