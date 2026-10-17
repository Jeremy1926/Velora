#include "pch.h"
#include "Engine/NetworkObjectList.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FNetworkObjectList::Remove(AActor* const Actor)
{
    static auto Remove = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("NotifyActorDestroyed %s - StartupActor"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        bool bNetworkObjects = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && bNetworkObjects)
                return (void (*)(FNetworkObjectList*, AActor*))(Address + Length + (int32_t)Instruction.imm.imm32);

            bNetworkObjects = Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 2;
            Address += Length;
        }

        return (void (*)(FNetworkObjectList*, AActor*))nullptr;
    }();

    Remove(this, Actor);
}
