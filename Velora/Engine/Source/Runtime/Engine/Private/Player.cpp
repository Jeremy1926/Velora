#include "pch.h"
#include "Engine/Player.h"
#include "UObject/UnrealType.h"

APlayerController* UPlayer::GetPlayerController() const
{
    static auto PlayerController = GetClass()->FindPropertyByName(FName(TEXT("PlayerController")));

    return *PlayerController->ContainerPtrToValuePtr<APlayerController*>(this);
}

int32 UPlayer::GetCurrentNetSpeed() const
{
    static auto CurrentNetSpeed = GetClass()->FindPropertyByName(FName(TEXT("CurrentNetSpeed")));

    return *CurrentNetSpeed->ContainerPtrToValuePtr<int32>(this);
}
