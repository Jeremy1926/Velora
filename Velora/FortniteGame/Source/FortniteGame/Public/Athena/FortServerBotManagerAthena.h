#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class AFortAthenaAIBotController;
class AFortGameModeAthena;
class AFortGameStateAthena;
class UAthenaAIPopulationTracker;
class UWorld;
class UScriptStruct;

struct FFortServerBotInfo
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    AFortAthenaAIBotController*& GetBotController();
};

template<>
class TArray<FFortServerBotInfo, FDefaultAllocator> : public TRuntimeSizedArray<FFortServerBotInfo>
{
};

class UFortServerBotManagerAthena : public UObject
{
public:
    static UClass* StaticClass();

    static UFortServerBotManagerAthena* GetServerBotManager(UWorld* World);

    void SetBotBrainActivationTimer();

    static void TickBotBrainActivationTimers();

    void CheckForBotBrainActivation();

    void AssignTeam(AFortAthenaAIBotController* BotController, const uint8* CustomSquadId);

    TArray<FFortServerBotInfo>* GetPlayerBots();

    TArray<FFortServerBotInfo>* GetNoneAthenaParticipantBots();

    AFortGameModeAthena*& GetCachedGameMode();

    AFortGameStateAthena*& GetCachedGameState();

    UAthenaAIPopulationTracker* GetCachedAIPopulationTracker();

    void SetCachedAIPopulationTracker(UAthenaAIPopulationTracker* InCachedAIPopulationTracker);
};
