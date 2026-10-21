#pragma once

#include "AI/FortAISystem.h"

class UAthenaAIPerceptionManager;
class UAthenaAIPopulationTracker;
class UAthenaAISpawner;

class UAthenaAISystem : public UFortAISystem
{
public:
    static UClass* StaticClass();

    UAthenaAIPerceptionManager* GetPerceptionManager() const;

    UAthenaAIPopulationTracker* GetAIPopulationTracker() const;

    UAthenaAISpawner* GetAISpawner() const;
};
