#include "pch.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

TArray<APlayerState*>& AGameStateBase::GetPlayerArray()
{
    static auto PlayerArray = GetClass()->FindPropertyByName(FName(TEXT("PlayerArray")));

    return *PlayerArray->ContainerPtrToValuePtr<TArray<APlayerState*>>(this);
}
