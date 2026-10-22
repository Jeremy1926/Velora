#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AFortAthenaAIBotController;
class AFortGameModeAthena;
class AFortGameStateAthena;
class AFortPlayerPawnAthena;
class UFortAthenaAIBotCustomizationData;
struct FFortAthenaAIBotRunTimeCustomizationData;
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

    AFortPlayerPawnAthena* SpawnBot(const FVector& InSpawnLocation, const FRotator& InSpawnRotation, const UFortAthenaAIBotCustomizationData* InBotData, const FFortAthenaAIBotRunTimeCustomizationData& InRuntimeBotData);

    static const UFortAthenaAIBotCustomizationData* SpawningBotData;

    static const FFortAthenaAIBotRunTimeCustomizationData* SpawningRuntimeBotData;

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
