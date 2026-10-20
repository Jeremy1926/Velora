#pragma once

#include "FortGamePvPBase.h"
#include "Templates/SubclassOf.h"

class AFortAthenaVehicle;
class AFortPlayerControllerAthena;
class UFortServerBotManagerAthena;

class UAthenaAISettings;

class AFortGameModeAthena : public AFortGamePvPBase
{
public:
    static UClass* StaticClass();

    void InitGameState();

    bool ReadyToStartMatch_Implementation();

    int32 CountReadyPlayers();

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

    void PlacePlayerOnTeam(AController* Player);

    void AddToAlivePlayers(AFortPlayerControllerAthena* PlayerController);

    void OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions);

    void OnPlaylistDataLoaded();

    void OverrideAISettings(UAthenaAISettings* InAISettings);

    UAthenaAISettings*& GetAISettings();

    UFortServerBotManagerAthena*& GetServerBotManager();

    TSubclassOf<UFortServerBotManagerAthena> GetServerBotManagerClass() const;

    int32& GetWarmupRequiredPlayerCount();

    bool GetDisableGCOnServerDuringMatch() const;

    FName RedirectLootTierGroup(FName LootTierGroup) const;

    TSubclassOf<AActor> GetVehicleClassOverride(TSubclassOf<AActor> DefaultVehicleClass) const;

    bool ShouldSpawnVehicle(TSubclassOf<AFortAthenaVehicle> VehicleClass);

    bool IsMapInfoInitialized() const;
};
