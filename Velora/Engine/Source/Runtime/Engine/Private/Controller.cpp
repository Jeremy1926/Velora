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

    return Scanner::Virtual<AActor* (*)(const AController*)>(this, GetViewTarget)(this);
}

FRotator AController::GetControlRotation() const
{
    static auto GetControlRotation = Scanner::Exec(TEXT("/Script/Engine.Controller:GetControlRotation")).Find("call [*+0x*]").Slot(TEXT("AController::GetControlRotation"));

    FRotator Result{};
    Scanner::Virtual<FRotator* (*)(const AController*, FRotator*)>(this, GetControlRotation)(this, &Result);

    return Result;
}

void AController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
    static auto GetPlayerViewPoint = []
    {
        auto GetActorEyesViewPoint = Scanner::Exec(TEXT("/Script/Engine.Actor:GetActorEyesViewPoint")).Find("call [*+0x*]").Slot(TEXT("AActor::GetActorEyesViewPoint"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Controller"));
        auto Forward = std::format("mov rax, [rcx]; jmp [rax+0x{:x}]", GetActorEyesViewPoint);

        return Scanner::Slot(TEXT("AController::GetPlayerViewPoint"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Is(Forward); });
    }();

    Scanner::Virtual<void (*)(const AController*, FVector&, FRotator&)>(this, GetPlayerViewPoint)(this, Location, Rotation);
}
