#pragma once

#include "HAL/Platform.h"

class UClass;

class IFortTeamActorInterface
{
public:
    static UClass* StaticClass();

    uint8 GetTeam();
};
