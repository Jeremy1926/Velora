#include "pch.h"
#include "UObject/Class.h"
#include "Scanner.h"
#include "hde/hde64.h"

static const uint8_t* FindClassDefaultObjectCheck()
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'.")));

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x83 && Instruction.rex_w && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 2 && Instruction.imm.imm8 == 0)
            return Address;

        Address += Length;
    }
}

UProperty* UStruct::FindPropertyByName(FName InName) const
{
    static auto FindPropertyByName = (UProperty* (*)(const UStruct*, FName))Scanner::FindPattern("48 8B 41 58 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 18 74 ? 48 8B 40 50 48 85 C0 75 ? 33 C0 C3");

    return FindPropertyByName(this, InName);
}

Native UFunction::GetNativeFunc() const
{
    static auto Func = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Failed to bind native function %s.%s"));
        auto Function = Scanner::FindFunction(Reference);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    return *(Native*)((uint8_t*)this + Func);
}

UObject* UClass::GetDefaultObject(bool bCreateIfNeeded) const
{
    static auto ClassDefaultObject = []
    {
        hde64s Instruction;
        hde64_disasm(FindClassDefaultObjectCheck(), &Instruction);

        return (int32_t)Instruction.disp.disp32;
    }();

    if (*(UObject**)((uint8_t*)this + ClassDefaultObject) == nullptr && bCreateIfNeeded)
    {
        InternalCreateDefaultObjectWrapper();
    }

    return *(UObject**)((uint8_t*)this + ClassDefaultObject);
}

UObject* UClass::InternalCreateDefaultObjectWrapper() const
{
    static auto CreateDefaultObject = []
    {
        for (auto Address = FindClassDefaultObjectCheck();;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(UObject* (**)(const UClass*))(VTable + CreateDefaultObject))(this);
}
