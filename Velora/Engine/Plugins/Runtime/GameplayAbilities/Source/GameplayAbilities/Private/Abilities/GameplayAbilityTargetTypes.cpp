#include "pch.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

void FAbilityReplicatedDataCache::Reset()
{
    static auto Reset = []
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));
        int32_t ServerSetReplicatedTargetDataCancelled_Implementation = 0;

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                ServerSetReplicatedTargetDataCancelled_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"));
        auto Address = VTable[ServerSetReplicatedTargetDataCancelled_Implementation / sizeof(void*)];

        for (auto Calls = 0;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 3)
                return (void (*)(FAbilityReplicatedDataCache*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    Reset(this);
}
