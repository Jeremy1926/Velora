#pragma once

#include "GameFramework/GameStateBase.h"
#include "Containers/UnrealString.h"

enum class EAthenaGamePhase : uint8
{
    None,
    Setup,
    Warmup,
    Aircraft,
    SafeZones,
    EndGame,
};

class AFortGameStateAthena : public AGameStateBase
{
public:
    static UClass* StaticClass();

    EAthenaGamePhase GetGamePhase() const;

    int32 GetPlayersLeft() const;

    int32 GetTeamsLeft() const;

    const FString& GetWinningPlayerName() const;
};
