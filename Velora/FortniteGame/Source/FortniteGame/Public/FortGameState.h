#pragma once

#include "GameFramework/GameStateBase.h"

enum class EFortGameplayState : uint8
{
    NormalGameplay,
    WaitingToStart,
    EndOfZone,
    EnteringZone,
    LeavingZone,
    Invalid
};

class AFortGameState : public AGameStateBase
{
public:
    static UClass* StaticClass();

    EFortGameplayState GetGameplayState() const;
};
