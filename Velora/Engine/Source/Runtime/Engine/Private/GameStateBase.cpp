#include "pch.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

TArray<APlayerState*>& AGameStateBase::GetPlayerArray()
{
    static auto PlayerArray = GetClass()->FindPropertyByName(FName(TEXT("PlayerArray")));

    return *PlayerArray->ContainerPtrToValuePtr<TArray<APlayerState*>>(this);
}

float AGameStateBase::GetServerWorldTimeSeconds() const
{
    static auto GetServerWorldTimeSeconds = Anchor::VirtualFunction<float (*)(const AGameStateBase*)>(TEXT("AGameStateBase::GetServerWorldTimeSeconds"))
        .Exec(TEXT("/Script/Engine.GameStateBase:GetServerWorldTimeSeconds"))
        .Find("call [*+0x*]")
        .Get();

    return GetServerWorldTimeSeconds(this);
}
