#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Scanner.h"
#include "hde/hde64.h"

struct FConsoleManagerCalls
{
    IConsoleManager** Singleton;
    const uint8_t* SetupSingleton;
    int32_t FindConsoleVariable;
    int32_t AsVariableInt;
};

static auto ConsoleManagerCalls = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")));
    auto End = Scanner::FindFunctionEnd(Function);
    FConsoleManagerCalls Calls = {};
    const uint8_t* Load = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End && !Calls.AsVariableInt;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm == 0x0D)
            Load = Address + Length + (int32_t)Instruction.disp.disp32;
        else if (!Calls.Singleton && Load && Instruction.opcode == 0x85 && Instruction.rex_w && Instruction.modrm == 0xC9)
            Calls.Singleton = (IConsoleManager**)Load;
        else if (Calls.Singleton && !Calls.SetupSingleton && Instruction.opcode == 0xE8)
            Calls.SetupSingleton = Address + Length + (int32_t)Instruction.imm.imm32;
        else if (Calls.SetupSingleton && Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
            Calls.FindConsoleVariable = (int32_t)Instruction.disp.disp32;
        else if (Calls.FindConsoleVariable && Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 1)
            Calls.AsVariableInt = (int8_t)Instruction.disp.disp8;

        Address += Length;
    }

    return Calls;
}();

IConsoleManager*& IConsoleManager::Singleton = *ConsoleManagerCalls.Singleton;

void IConsoleManager::SetupSingleton()
{
    static auto SetupSingleton = (void (*)())ConsoleManagerCalls.SetupSingleton;

    SetupSingleton();
}

IConsoleVariable* IConsoleManager::FindConsoleVariable(const TCHAR* Name) const
{
    auto VTable = *(uint8_t**)this;

    return (*(IConsoleVariable* (**)(const IConsoleManager*, const TCHAR*))(VTable + ConsoleManagerCalls.FindConsoleVariable))(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    auto VTable = *(uint8_t**)this;

    return (*(TConsoleVariableData<int32>* (**)(IConsoleObject*))(VTable + ConsoleManagerCalls.AsVariableInt))(this);
}

int32 IConsoleVariable::GetInt() const
{
    static auto GetInt = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("athena.Vehicles"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        bool bFindConsoleVariable = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2 && (int32_t)Instruction.disp.disp32 == ConsoleManagerCalls.FindConsoleVariable)
                bFindConsoleVariable = true;
            else if (bFindConsoleVariable && Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 1)
                return (int32_t)(int8_t)Instruction.disp.disp8;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const IConsoleVariable*))(VTable + GetInt))(this);
}
