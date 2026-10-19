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
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__PlayerController"));
        auto Body = std::format("push *; sub rsp, *; mov *, [rcx+0x{:x}]; cmp [rcx+0x{:x}], *; jz *; cmp qword ptr [rcx+0x{:x}], 0x0", Pawn, AcknowledgedPawn, SpectatorPawn);

        return Scanner::Slot(TEXT("APlayerController::SendClientAdjustment"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Is(Body); });
    }();

    Scanner::Virtual<void (*)(APlayerController*)>(this, SendClientAdjustment)(this);
}

void APlayerController::ServerAcknowledgePossession_Implementation(APawn* P)
{
    static auto ServerAcknowledgePossession = Scanner::Virtual<void (*)(APlayerController*, APawn*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__PlayerController")), Scanner::Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession")).Find("call [rax+*]", 2).Slot(TEXT("APlayerController::ServerAcknowledgePossession_Implementation")));

    ServerAcknowledgePossession(this, P);
}
