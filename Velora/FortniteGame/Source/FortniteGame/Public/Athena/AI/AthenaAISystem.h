#pragma once

#include "AI/FortAISystem.h"

class UAthenaAIPerceptionManager;
class UFortServerBotManagerAthena;

class UAthenaAISystem : public UFortAISystem
{
public:
    static UClass* StaticClass();

    UAthenaAIPerceptionManager* GetPerceptionManager() const;

    UFortServerBotManagerAthena*& GetPlayerBotManager();
};
