#pragma once

#include "GameFramework/GameStateBase.h"

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
};
