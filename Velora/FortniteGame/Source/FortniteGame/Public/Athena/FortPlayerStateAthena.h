#pragma once

#include "Player/FortPlayerStatePvP.h"

class AFortPlayerStateAthena : public AFortPlayerStatePvP
{
public:
    static UClass* StaticClass();

    uint8 GetSquadId() const;

    uint8 GetTeamIndex() const;

    void PlayerPawnRebooted();

    int32& GetRebootCounter();
};
