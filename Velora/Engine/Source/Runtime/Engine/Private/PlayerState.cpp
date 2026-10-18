#include "pch.h"
#include "GameFramework/PlayerState.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

FString APlayerState::GetPlayerName() const
{
    static auto PlayerName = GetClass()->FindPropertyByName(FName(TEXT("PlayerName")));

    return *PlayerName->ContainerPtrToValuePtr<FString>(this);
}
