#include "pch.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void APlayerController::SendClientAdjustment()
{
    static auto SendClientAdjustment = [this]
    {
        auto Pawn = GetClass()->FindPropertyByName(FName(TEXT("Pawn")))->GetOffset_ForInternal();
        auto AcknowledgedPawn = GetClass()->FindPropertyByName(FName(TEXT("AcknowledgedPawn")))->GetOffset_ForInternal();
        auto SpectatorPawn = GetClass()->FindPropertyByName(FName(TEXT("SpectatorPawn")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__PlayerController"));
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Is(std::format("push *; sub rsp, *; mov *, [rcx+0x{:x}]; cmp [rcx+0x{:x}], *; jz *; cmp qword ptr [rcx+0x{:x}], 0x0", Pawn, AcknowledgedPawn, SpectatorPawn)))
            Index++;

        return Scanner::Slot(TEXT("APlayerController::SendClientAdjustment"), Index * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(APlayerController*))(VTable + SendClientAdjustment))(this);
}
