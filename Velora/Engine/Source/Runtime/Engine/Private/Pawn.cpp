#include "pch.h"
#include "GameFramework/Pawn.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

AController* APawn::GetController() const
{
    static auto Controller = GetClass()->FindPropertyByName(FName(TEXT("Controller")));

    return *Controller->ContainerPtrToValuePtr<AController*>(this);
}

APlayerState* APawn::GetPlayerState() const
{
    static auto PlayerState = GetClass()->FindPropertyByName(FName(TEXT("PlayerState")));

    return *PlayerState->ContainerPtrToValuePtr<APlayerState*>(this);
}
