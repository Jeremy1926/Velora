#include "pch.h"
#include "FortGameModeZone.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto FinishWorldInitialization = DetourCall([]
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")));
    const uint8_t* PvPFinishWorldInitialization = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; !PvPFinishWorldInitialization;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            PvPFinishWorldInitialization = Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }

    for (auto Address = PvPFinishWorldInitialization;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            return Address;

        Address += Length;
    }
}(), &AFortGameModeZone::FinishWorldInitialization);

void AFortGameModeZone::FinishWorldInitialization()
{
    Super::FinishWorldInitialization();
}
