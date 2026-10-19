#pragma once

#include "FortGameStateZone.h"

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

class AFortGameStateAthena : public AFortGameStateZone
{
public:
    static UClass* StaticClass();

    EAthenaGamePhase GetGamePhase() const;

    AFortAthenaMapInfo* GetMapInfo() const;

    void SetCurrentPlaylistName(FName PlaylistName);

    void OnRep_CurrentPlaylistData();

    void StreamAdditionalPlaylistLevels();
};
