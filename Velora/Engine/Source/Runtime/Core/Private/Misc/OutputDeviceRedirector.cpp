#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* OutputDevice)
{
    static auto AddOutputDevice = []
    {
        auto Address = Scanner::FindStringReference(TEXT("NOCONSOLE"));

        for (auto Count = 0; Count < 3;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Count == 3)
                return (void (*)(FOutputDeviceRedirector*, FOutputDevice*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }

        return (void (*)(FOutputDeviceRedirector*, FOutputDevice*))nullptr;
    }();

    AddOutputDevice(this, OutputDevice);
}
