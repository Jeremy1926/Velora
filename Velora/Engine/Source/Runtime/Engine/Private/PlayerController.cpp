#include "pch.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CheatManager.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void APlayerController::SendClientAdjustment()
{
    static auto SendClientAdjustment = Anchor::VirtualFunction<void (*)(APlayerController*)>(TEXT("APlayerController::SendClientAdjustment"))
        .FieldVirtual(TEXT("/Script/Engine.Default__PlayerController"), "push *; sub rsp, *; mov *, [rcx+%x]; cmp [rcx+%x], *; jz *; cmp qword ptr [rcx+%x], 0x0", GetClass()->FindPropertyByName(FName(TEXT("Pawn"))), GetClass()->FindPropertyByName(FName(TEXT("AcknowledgedPawn"))), GetClass()->FindPropertyByName(FName(TEXT("SpectatorPawn"))))
        .Or()
        .Event(TEXT("ClientRecvServerAckFrame"))
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__PlayerController"))
        .Or()
        .Event(TEXT("ClientRecvServerAckFrame"))
        .InVirtual(TEXT("/Script/Engine.Default__PlayerController"), TEXT("APlayerController::SendClientAdjustment"))
        .Optional()
        .Get();

    if (SendClientAdjustment)
        return SendClientAdjustment(this);

    static auto NetworkPredictionInterfaceClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.NetworkPredictionInterface"));

    APawn* RemotePawn = GetPawnOrSpectator();

    if (RemotePawn && RemotePawn->GetRemoteRole() == ROLE_AutonomousProxy && !IsNetMode(NM_Client))
    {
        auto MovementComponent = RemotePawn->GetMovementComponent();
        auto NetworkPredictionInterface = MovementComponent ? MovementComponent->GetInterfaceAddress(NetworkPredictionInterfaceClass) : nullptr;

        if (NetworkPredictionInterface)
            (*(void (***)(void*))NetworkPredictionInterface)[2](NetworkPredictionInterface);
    }
}

void APlayerController::ServerAcknowledgePossession_Implementation(APawn* P)
{
    static auto ServerAcknowledgePossession = Anchor::VirtualFunction<void (*)(APlayerController*, APawn*)>(TEXT("APlayerController::ServerAcknowledgePossession_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+*]", 2)
        .Or()
        .Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+*]", 2)
        .Or()
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

void APlayerController::Reset()
{
    static auto Reset = Anchor::VirtualFunction<void (*)(APlayerController*)>(TEXT("APlayerController::Reset"))
        .Event(TEXT("K2_OnReset"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Or()
        .String(TEXT("K2_OnReset"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("mov ?dx, *; mov rbx, [rax+0x*]; call 0x*; xor r8d, r8d; mov rdx, rax; mov rcx, rdi; mov rax, rbx")
        .Start()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Get();

    if (Reset)
        Reset(this);
}

void APlayerController::ClientGotoState(FName NewState)
{
    static auto ClientGotoState = Anchor::Function<void (*)(APlayerController*, FName)>(TEXT("APlayerController::ClientGotoState"))
        .Event(TEXT("ClientGotoState"))
        .Is("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rax, [rcx]; mov rdi, rcx; mov [rsp+0x38], edx; mov edx, [0x*]")
        .Or()
        .Event(TEXT("ClientGotoState"))
        .Is("push rbx; sub rsp, 0x20; mov [rsp+0x38], edx; mov rbx, rcx; mov edx, [0x*]")
        .Get();

    if (ClientGotoState)
        ClientGotoState(this, NewState);
}

APawn* APlayerController::GetAcknowledgedPawn() const
{
    static auto AcknowledgedPawn = GetClass()->FindPropertyByName(FName(TEXT("AcknowledgedPawn")));

    return *AcknowledgedPawn->ContainerPtrToValuePtr<APawn*>(this);
}

APawn* APlayerController::GetSpectatorPawn() const
{
    static auto SpectatorPawn = GetClass()->FindPropertyByName(FName(TEXT("SpectatorPawn")));

    return *SpectatorPawn->ContainerPtrToValuePtr<APawn*>(this);
}

APawn* APlayerController::GetPawnOrSpectator() const
{
    return GetPawn() ? GetPawn() : GetSpectatorPawn();
}

UCheatManager* APlayerController::GetCheatManager() const
{
    static auto CheatManager = GetClass()->FindPropertyByName(FName(TEXT("CheatManager")));

    return *CheatManager->ContainerPtrToValuePtr<UCheatManager*>(this);
}

void APlayerController::SetCheatManager(UCheatManager* InCheatManager)
{
    static auto CheatManager = GetClass()->FindPropertyByName(FName(TEXT("CheatManager")));

    *CheatManager->ContainerPtrToValuePtr<UCheatManager*>(this) = InCheatManager;
}

UClass* APlayerController::GetCheatClass() const
{
    static auto CheatClass = GetClass()->FindPropertyByName(FName(TEXT("CheatClass")));

    return *CheatClass->ContainerPtrToValuePtr<UClass*>(this);
}

void APlayerController::AddCheats(bool bForce)
{
    if (GetCheatManager() || !GetCheatClass() || !bForce)
        return;

    SetCheatManager(NewObject<UCheatManager>(this, GetCheatClass()));
}
