#include "pch.h"
#include "GameFramework/Controller.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static int32_t FindExecSlot(const TCHAR* FunctionName)
{
    for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, FunctionName))->GetNativeFunc());;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
            return (int32_t)Instruction.disp.disp32;

        Address += Length;
    }
}

APawn* AController::GetPawn() const
{
    static auto Pawn = GetClass()->FindPropertyByName(FName(TEXT("Pawn")));

    return *Pawn->ContainerPtrToValuePtr<APawn*>(this);
}

AActor* AController::GetViewTarget() const
{
    static auto GetViewTarget = FindExecSlot(TEXT("/Script/Engine.Controller:GetViewTarget"));

    auto VTable = *(uint8_t**)this;

    return (*(AActor* (**)(const AController*))(VTable + GetViewTarget))(this);
}

FRotator AController::GetControlRotation() const
{
    static auto GetControlRotation = FindExecSlot(TEXT("/Script/Engine.Controller:GetControlRotation"));

    auto VTable = *(uint8_t**)this;
    FRotator Result;

    return *(*(FRotator* (**)(const AController*, FRotator*))(VTable + GetControlRotation))(this, &Result);
}

void AController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
    static auto GetPlayerViewPoint = []
    {
        auto GetActorEyesViewPoint = FindExecSlot(TEXT("/Script/Engine.Actor:GetActorEyesViewPoint"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Controller"));

        for (auto Index = 0;; Index++)
        {
            hde64s Load;
            auto Length = hde64_disasm(VTable[Index], &Load);

            if (Load.opcode == 0x8B && Load.rex_w && Load.modrm == 0x01)
            {
                hde64s Jump;
                hde64_disasm(VTable[Index] + Length, &Jump);

                if (Jump.opcode == 0xFF && Jump.modrm_reg == 4 && Jump.modrm_mod == 2 && (int32_t)Jump.disp.disp32 == GetActorEyesViewPoint)
                    return Index * (int32_t)sizeof(void*);
            }
        }
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(const AController*, FVector&, FRotator&))(VTable + GetPlayerViewPoint))(this, Location, Rotation);
}
