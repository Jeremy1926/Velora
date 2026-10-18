#include "pch.h"
#include "GameFramework/Controller.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

APawn* AController::GetPawn() const
{
    static auto Pawn = GetClass()->FindPropertyByName(FName(TEXT("Pawn")));

    return *Pawn->ContainerPtrToValuePtr<APawn*>(this);
}

APlayerState* AController::GetPlayerState() const
{
    static auto PlayerState = GetClass()->FindPropertyByName(FName(TEXT("PlayerState")));

    return *PlayerState->ContainerPtrToValuePtr<APlayerState*>(this);
}

AActor* AController::GetViewTarget() const
{
    static auto GetViewTarget = Scanner::Exec(TEXT("/Script/Engine.Controller:GetViewTarget")).Find("call [*+0x*]").Slot(TEXT("AController::GetViewTarget"));

    auto VTable = *(uint8_t**)this;

    return (*(AActor* (**)(const AController*))(VTable + GetViewTarget))(this);
}

FRotator AController::GetControlRotation() const
{
    static auto GetControlRotation = Scanner::Exec(TEXT("/Script/Engine.Controller:GetControlRotation")).Find("call [*+0x*]").Slot(TEXT("AController::GetControlRotation"));

    auto VTable = *(uint8_t**)this;
    FRotator Result;

    return *(*(FRotator* (**)(const AController*, FRotator*))(VTable + GetControlRotation))(this, &Result);
}

void AController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
    static auto GetPlayerViewPoint = []
    {
        auto GetActorEyesViewPoint = Scanner::Exec(TEXT("/Script/Engine.Actor:GetActorEyesViewPoint")).Find("call [*+0x*]").Slot(TEXT("AActor::GetActorEyesViewPoint"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Controller"));
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Is(std::format("mov rax, [rcx]; jmp [rax+0x{:x}]", GetActorEyesViewPoint)))
            Index++;

        return Scanner::Slot(TEXT("AController::GetPlayerViewPoint"), Index * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(const AController*, FVector&, FRotator&))(VTable + GetPlayerViewPoint))(this, Location, Rotation);
}
