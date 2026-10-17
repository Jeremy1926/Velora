#pragma once

#include "GameFramework/GameModeBase.h"

namespace MatchState
{
    extern const FName WaitingToStart;
}

class AGameMode : public AGameModeBase
{
public:
    FName GetMatchState() const;
};
