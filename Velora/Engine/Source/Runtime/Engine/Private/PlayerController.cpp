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
        auto Body = std::format("push *; sub rsp, *; mov *, [rcx+0x{:x}]; cmp [rcx+0x{:x}], *; jz *; cmp qword ptr [rcx+0x{:x}], 0x0", Pawn, AcknowledgedPawn, SpectatorPawn);

        return Anchor::Slot(TEXT("APlayerController::SendClientAdjustment"))
            .Virtual(TEXT("/Script/Engine.Default__PlayerController"), [&](const Anchor::FCode& Entry) { return Entry.Is(Body); })
            .Get();
    }();

    Scanner::Virtual<void (*)(APlayerController*)>(this, SendClientAdjustment)(this);
}

void APlayerController::ServerAcknowledgePossession_Implementation(APawn* P)
{
    static auto ServerAcknowledgePossessionSlot = Anchor::Slot(TEXT("APlayerController::ServerAcknowledgePossession_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession"))
        .Find("call [rax+*]", 2)
        .Get();

    static auto ServerAcknowledgePossession = Scanner::Virtual<void (*)(APlayerController*, APawn*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__PlayerController")), ServerAcknowledgePossessionSlot);

    ServerAcknowledgePossession(this, P);
}

void APlayerController::ServerRestartPlayer_Implementation()
{
    static auto ServerRestartPlayer = Anchor::Slot(TEXT("APlayerController::ServerRestartPlayer_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer"))
        .Find("jmp [rax+*]")
        .Get();

    Scanner::Virtual<void (*)(APlayerController*)>(this, ServerRestartPlayer)(this);
}
