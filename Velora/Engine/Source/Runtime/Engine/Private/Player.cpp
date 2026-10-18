#include "pch.h"
#include "Engine/Player.h"
#include "UObject/UnrealType.h"

APlayerController* UPlayer::GetPlayerController() const
{
    static auto PlayerController = GetClass()->FindPropertyByName(FName(TEXT("PlayerController")));

    return *PlayerController->ContainerPtrToValuePtr<APlayerController*>(this);
}
