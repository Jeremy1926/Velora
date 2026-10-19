#pragma once

#include "GameFramework/GameStateBase.h"

class AFortAthenaMapInfo;

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

    AFortAthenaMapInfo* GetMapInfo() const;
};
