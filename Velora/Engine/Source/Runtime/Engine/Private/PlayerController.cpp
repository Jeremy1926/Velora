#include "pch.h"
#include "GameFramework/PlayerController.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void APlayerController::SendClientAdjustment()
{
    static auto SendClientAdjustment = Anchor::VirtualFunction<void (*)(APlayerController*)>(TEXT("APlayerController::SendClientAdjustment"))
        .FieldVirtual(TEXT("/Script/Engine.Default__PlayerController"), "push *; sub rsp, *; mov *, [rcx+%x]; cmp [rcx+%x], *; jz *; cmp qword ptr [rcx+%x], 0x0", GetClass()->FindPropertyByName(FName(TEXT("Pawn"))), GetClass()->FindPropertyByName(FName(TEXT("AcknowledgedPawn"))), GetClass()->FindPropertyByName(FName(TEXT("SpectatorPawn"))))
        .Get();

    SendClientAdjustment(this);
}

void APlayerController::ServerAcknowledgePossession_Implementation(APawn* P)
{
    static auto ServerAcknowledgePossession = Anchor::VirtualFunction<void (*)(APlayerController*, APawn*)>(TEXT("APlayerController::ServerAcknowledgePossession_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession"))
        .Find("call [rax+*]", 2)
        .Get(TEXT("/Script/Engine.Default__PlayerController"));

    ServerAcknowledgePossession(this, P);
}

void APlayerController::ServerRestartPlayer_Implementation()
{
    static auto ServerRestartPlayer = Anchor::VirtualFunction<void (*)(APlayerController*)>(TEXT("APlayerController::ServerRestartPlayer_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer"))
        .Find("jmp [rax+*]")
        .Get();

    ServerRestartPlayer(this);
}
