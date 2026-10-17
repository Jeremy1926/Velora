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

static auto GameThreadGlobals = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")));
    auto End = Scanner::FindFunctionEnd(Function);
    std::array<const uint8_t*, 3> Globals = {};
    auto Count = 1ull;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End && Count < Globals.size();)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);
        auto Target = Address + Length + (int32_t)Instruction.disp.disp32;

        if (Instruction.opcode == 0x80 && Instruction.modrm == 0x3D && Instruction.imm.imm8 == 0)
            Globals[0] = Target;
        else if (Globals[0] && Instruction.opcode == 0x3B && (Instruction.modrm & 0xC7) == 0x05)
            Globals[Count++] = Target;

        Address += Length;
    }

    return Globals;
}();

bool& GIsGameThreadIdInitialized = *(bool*)GameThreadGlobals[0];
uint32& GGameThreadId = *(uint32*)GameThreadGlobals[1];
uint32& GSlateLoadingThreadId = *(uint32*)GameThreadGlobals[2];

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
