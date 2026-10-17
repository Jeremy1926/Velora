#include "pch.h"
#include "Scanner.h"
#include "hde/hde64.h"

bool& GIsAllowingParticles = *[]
{
    for (auto Address = Scanner::FindStringReference(TEXT("CreateRenderState_Concurrent @ %fs %s"));;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x80 && Instruction.modrm == 0x3D && Instruction.imm.imm8 == 0)
            return (bool*)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }
}();
