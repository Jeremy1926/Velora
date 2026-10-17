#include "pch.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* UObject::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Object"));

    return Class;
}

UWorld* UObject::GetWorld() const
{
    static auto GetWorld = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Changing GameSessionId from '%s' to '%s'")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(UWorld* (**)(const UObject*))(VTable + GetWorld))(this);
}

UObject* UObject::GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags)
{
    static auto GetArchetypeFromRequiredInfo = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Component archetype is not the CDO nor a default subobject of my class. Me = %s, Component = %s, Archetype = %s"));
        auto Function = Scanner::FindFunction(Reference);
        std::array<hde64s, 4> Loads = {};
        const uint8_t* Call = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 &&
                Loads[0].opcode == 0x8B && Loads[0].rex_r && !Loads[0].rex_w && Loads[0].modrm_reg == 1 &&
                Loads[1].opcode == 0x8B && Loads[1].rex_r && Loads[1].rex_w && Loads[1].modrm_reg == 0 &&
                Loads[2].opcode == 0x8B && !Loads[2].rex_r && Loads[2].rex_w && Loads[2].modrm_reg == 2 &&
                Loads[3].opcode == 0x8B && !Loads[3].rex_r && Loads[3].rex_w && Loads[3].modrm_reg == 1)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            std::shift_left(Loads.begin(), Loads.end(), 1);
            Loads.back() = Instruction;
            Address += Length;
        }

        return (UObject* (*)(UClass*, UObject*, FName, EObjectFlags))Call;
    }();

    return GetArchetypeFromRequiredInfo(Class, Outer, Name, ObjectFlags);
}

UFunction* UObject::FindFunctionChecked(FName InName) const
{
    static auto FindFunctionChecked = (UFunction* (*)(const UObject*, FName))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Failed to find function %s in %s")))->BeginAddress);

    return FindFunctionChecked(this, InName);
}

void UObject::ProcessEvent(UFunction* Function, void* Parms)
{
    static auto ProcessEvent = []
    {
        auto NotifyEventTriggered = Scanner::FindFunction(Scanner::FindStringReference(TEXT("NotifyEventTriggered: Unable to find function '%s'")));
        auto End = Scanner::FindFunctionEnd(NotifyEventTriggered);

        for (auto Address = (const uint8_t*)Scanner::Base() + NotifyEventTriggered->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(UObject*, UFunction*, void*))(VTable + ProcessEvent))(this, Function, Parms);
}
