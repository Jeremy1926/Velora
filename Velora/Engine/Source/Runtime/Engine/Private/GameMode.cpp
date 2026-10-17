#include "pch.h"
#include "GameFramework/GameMode.h"
#include "UObject/UnrealType.h"

const FName MatchState::WaitingToStart = FName(TEXT("WaitingToStart"));

FName AGameMode::GetMatchState() const
{
    static auto MatchState = GetClass()->FindPropertyByName(FName(TEXT("MatchState")));

    return *MatchState->ContainerPtrToValuePtr<FName>(this);
}
