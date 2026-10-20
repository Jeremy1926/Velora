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
    static auto GetViewTarget = Anchor::Slot(TEXT("AController::GetViewTarget"))
        .Exec(TEXT("/Script/Engine.Controller:GetViewTarget"))
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<AActor* (*)(const AController*)>(this, GetViewTarget)(this);
}

FRotator AController::GetControlRotation() const
{
    static auto GetControlRotation = Anchor::Slot(TEXT("AController::GetControlRotation"))
        .Exec(TEXT("/Script/Engine.Controller:GetControlRotation"))
        .Find("call [*+0x*]")
        .Get();

    FRotator Result{};
    Scanner::Virtual<FRotator* (*)(const AController*, FRotator*)>(this, GetControlRotation)(this, &Result);

    return Result;
}

void AController::SetControlRotation(const FRotator& NewRotation)
{
    static auto SetControlRotation = Anchor::Slot(TEXT("AController::SetControlRotation"))
        .Exec(TEXT("/Script/Engine.Controller:SetControlRotation"))
        .Find("call [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(AController*, const FRotator&)>(this, SetControlRotation)(this, NewRotation);
}

FName& AController::GetStateName()
{
    static auto StateName = GetClass()->FindPropertyByName(FName(TEXT("StateName")));

    return *StateName->ContainerPtrToValuePtr<FName>(this);
}

void AController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
    static auto GetPlayerViewPoint = []
    {
        auto GetActorEyesViewPoint = Anchor::Slot(TEXT("AActor::GetActorEyesViewPoint"))
            .Exec(TEXT("/Script/Engine.Actor:GetActorEyesViewPoint"))
            .Find("call [*+0x*]")
            .Get();
        auto Forward = std::format("mov rax, [rcx]; jmp [rax+0x{:x}]", GetActorEyesViewPoint);

        return Anchor::Slot(TEXT("AController::GetPlayerViewPoint"))
            .Virtual(TEXT("/Script/Engine.Default__Controller"), [&](const Anchor::FCode& Entry) { return Entry.Is(Forward); })
            .Get();
    }();

    Scanner::Virtual<void (*)(const AController*, FVector&, FRotator&)>(this, GetPlayerViewPoint)(this, Location, Rotation);
}
