#pragma once

#include "UObject/Object.h"

class APawn;

class UFortAthenaAISpawnerDataComponent_AIBotConversation : public UObject
{
public:
    void OnSpawned(APawn* SpawnedPawn);
};
