#include "pch.h"
#include "GameFramework/Controller.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Controller"));

    return Class;
}

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

FMulticastScriptDelegate& AController::GetOnPossessedPawnChanged()
{
    static auto OnPossessedPawnChanged = StaticClass()->FindPropertyByName(FName(TEXT("OnPossessedPawnChanged")));

    return *OnPossessedPawnChanged->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

AActor* AController::GetViewTarget() const
{
    static auto GetViewTarget = Anchor::VirtualFunction<AActor* (*)(const AController*)>(TEXT("AController::GetViewTarget"))
        .Exec(TEXT("/Script/Engine.Controller:GetViewTarget"))
        .Find("call [*+0x*]")
        .Get();

    return GetViewTarget(this);
}

FRotator AController::GetControlRotation() const
{
    static auto GetControlRotation = Anchor::VirtualFunction<FRotator* (*)(const AController*, FRotator*)>(TEXT("AController::GetControlRotation"))
        .Exec(TEXT("/Script/Engine.Controller:GetControlRotation"))
        .Find("call [*+0x*]")
        .Get();

    FRotator Result{};
    GetControlRotation(this, &Result);

    return Result;
}

void AController::SetControlRotation(const FRotator& NewRotation)
{
    static auto SetControlRotation = Anchor::VirtualFunction<void (*)(AController*, const FRotator&)>(TEXT("AController::SetControlRotation"))
        .Exec(TEXT("/Script/Engine.Controller:SetControlRotation"))
        .Find("call [*+0x*]")
        .Get();

    SetControlRotation(this, NewRotation);
}

FName AController::GetStateName()
{
    static auto StateName = GetClass()->FindPropertyByName(FName(TEXT("StateName")));

    return StateName->GetPropertyValue_InContainer<FName>(this);
}

void AController::SetStateName(FName InStateName)
{
    static auto StateName = GetClass()->FindPropertyByName(FName(TEXT("StateName")));

    StateName->SetPropertyValue_InContainer<FName>(this, InStateName);
}

void AController::GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const
{
    static auto GetPlayerViewPoint = Anchor::VirtualFunction<void (*)(const AController*, FVector&, FRotator&)>(TEXT("AController::GetPlayerViewPoint"))
        .Exec(TEXT("/Script/Engine.Actor:GetActorEyesViewPoint"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AActor::GetActorEyesViewPoint"))
        .ForwarderIn(TEXT("/Script/Engine.Default__Controller"))
        .Get();

    GetPlayerViewPoint(this, Location, Rotation);
}
