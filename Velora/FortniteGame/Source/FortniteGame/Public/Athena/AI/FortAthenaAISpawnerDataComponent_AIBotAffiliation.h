#pragma once

#include "UObject/Object.h"

class APawn;

class UFortAthenaAISpawnerDataComponent_AIBotAffiliation : public UObject
{
public:
    void OnSpawned(APawn* SpawnedPawn);

    bool GetSquadID(int32* OutSquadId);
};
