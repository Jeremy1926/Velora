#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class AFortAthenaAIBotController;
class AFortGameModeAthena;
class AFortGameStateAthena;
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

    void SetBotBrainActivationTimer();

    static void TickBotBrainActivationTimers();

    void CheckForBotBrainActivation();

    TArray<FFortServerBotInfo>& GetPlayerBots();

    TArray<FFortServerBotInfo>& GetNoneAthenaParticipantBots();

    AFortGameModeAthena*& GetCachedGameMode();

    AFortGameStateAthena*& GetCachedGameState();
};
