#include "pch.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
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

    static auto bDoubleTime = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.GameStateBase:GetServerWorldTimeSeconds")))->FindPropertyByName(FName(TEXT("ReturnValue")))->GetSize() == (int32)sizeof(double);

    return bDoubleTime ? (float)GetServerWorldTimeSeconds.As<double (*)(const AGameStateBase*)>()(this) : GetServerWorldTimeSeconds(this);
}
