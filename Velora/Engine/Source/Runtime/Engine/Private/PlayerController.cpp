#include "pch.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

void APlayerController::SendClientAdjustment()
{
    static auto SendClientAdjustment = [this]
    {
        auto Pawn = GetClass()->FindPropertyByName(FName(TEXT("Pawn")))->GetOffset_ForInternal();
        auto AcknowledgedPawn = GetClass()->FindPropertyByName(FName(TEXT("AcknowledgedPawn")))->GetOffset_ForInternal();
        auto SpectatorPawn = GetClass()->FindPropertyByName(FName(TEXT("SpectatorPawn")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__PlayerController"));

        for (auto Index = 0;; Index++)
        {
            bool bPawn = false;
            bool bAcknowledgedPawn = false;
            auto Address = VTable[Index];

            for (auto Count = 0; Count < 6; Count++)
            {
                hde64s Instruction;
                auto Length = hde64_disasm(Address, &Instruction);

                if (Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm_mod == 2 && Instruction.modrm_rm == 1 && (int32_t)Instruction.disp.disp32 == Pawn)
                    bPawn = true;
                else if (bPawn && Instruction.opcode == 0x39 && Instruction.rex_w && Instruction.modrm_mod == 2 && Instruction.modrm_rm == 1 && (int32_t)Instruction.disp.disp32 == AcknowledgedPawn)
                    bAcknowledgedPawn = true;
                else if (bAcknowledgedPawn && Instruction.opcode == 0x83 && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 2 && Instruction.modrm_rm == 1 && (int32_t)Instruction.disp.disp32 == SpectatorPawn && Instruction.imm.imm8 == 0)
                    return Index * (int32_t)sizeof(void*);

                Address += Length;
            }
        }
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(APlayerController*))(VTable + SendClientAdjustment))(this);
}
