#pragma once

#include "UObject/Object.h"

class AFortGameModeAthena;
class AFortGameStateAthena;

class UFortServerBotManagerAthena : public UObject
{
public:
    static UClass* StaticClass();

    AFortGameModeAthena*& GetCachedGameMode();

    AFortGameStateAthena*& GetCachedGameState();
};
