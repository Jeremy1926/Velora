#include "pch.h"
#include "FortRegisteredPlayerInfo.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto AthenaProfileQuery_Completed = Detour([]
{
    auto Reference = Scanner::FindStringReference(TEXT("Initialized MCP profile for %s"));
    auto Function = Scanner::FindFunction(Reference);
    const uint8_t* SynchronizeProfile = nullptr;
    auto bForce = false;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (bForce && Instruction.opcode == 0xE8)
            SynchronizeProfile = Address + Length + (int32_t)Instruction.imm.imm32;

        bForce = Instruction.opcode == 0xB2 && Instruction.imm.imm8 == 1;
        Address += Length;
    }

    auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(SynchronizeProfile));
    const uint8_t* Method = nullptr;
    const uint8_t* Completed = nullptr;

    for (auto Address = SynchronizeProfile; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Method && Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 1 && Instruction.disp.disp8 == 0x10)
            Completed = Method;

        Method = Instruction.opcode == 0x8D && Instruction.rex_w && Instruction.modrm == 0x05 ? Address + Length + (int32_t)Instruction.disp.disp32 : nullptr;
        Address += Length;
    }

    return Completed;
}(), &UFortRegisteredPlayerInfo::AthenaProfileQuery_Completed);

void UFortRegisteredPlayerInfo::AthenaProfileQuery_Completed(const FMcpQueryResult& Result, int32 RequestId)
{
    (this->*::AthenaProfileQuery_Completed)(Result, RequestId);

    UpdateAbilitySystemActor(true);
}

AActor* UFortRegisteredPlayerInfo::UpdateAbilitySystemActor(bool bForceUpdate)
{
    static auto UpdateAbilitySystemActor = (AActor* (*)(UFortRegisteredPlayerInfo*, bool))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]")))->BeginAddress);

    return UpdateAbilitySystemActor(this, bForceUpdate);
}
