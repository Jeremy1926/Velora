#pragma once

#include "AISubsystem.h"
#include "Athena/FortServerBotManagerAthena.h"

class UAthenaAIPopulationTracker : public UAISubsystem
{
public:
    static UClass* StaticClass();

    TArray<FFortServerBotInfo>* GetPlayerBots();

    TArray<FFortServerBotInfo>* GetNoneAthenaParticipantBots();
};
