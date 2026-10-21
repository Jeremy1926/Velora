#pragma once

#include "Athena/AI/FortAthenaAISpawnerDataComponent_SkillsetBase.h"

class APawn;

class UFortAthenaAISpawnerDataComponent_AIBotSkillset : public UFortAthenaAISpawnerDataComponent_SkillsetBase
{
public:
    void OnSpawned(APawn* SpawnedPawn);
};
