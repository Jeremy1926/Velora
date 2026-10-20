#pragma once

#include "Player/FortPlayerStatePvP.h"

class AFortPlayerStateAthena : public AFortPlayerStatePvP
{
public:
    static UClass* StaticClass();

    uint8 GetSquadId() const;
};
