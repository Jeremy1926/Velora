#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto CommandletFlags = []
{
    auto Reference = Scanner::FindStringReference(TEXT("Cannot run editor commandlet %s with game executable."));
    auto Function = Scanner::FindFunction(Reference);
    std::array<bool*, 2> Flags = {};

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x88 && Instruction.modrm == 0x05)
        {
            Flags[0] = Flags[1];
            Flags[1] = (bool*)(Address + Length + (int32_t)Instruction.disp.disp32);
        }

        Address += Length;
    }

    return Flags;
}();

bool& GIsClient = *CommandletFlags[0];
bool& GIsServer = *CommandletFlags[1];

FOutputDeviceRedirector* GetGlobalLogSingleton()
{
    static auto GetGlobalLogSingleton = []
    {
        auto Address = Scanner::FindStringReference(TEXT("NOCONSOLE"));

        for (auto Count = 0; Count < 2;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Count == 2)
                return (FOutputDeviceRedirector* (*)())(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }

        return (FOutputDeviceRedirector* (*)())nullptr;
    }();

    return GetGlobalLogSingleton();
}
