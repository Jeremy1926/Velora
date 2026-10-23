#pragma once

#include "FortGamePvPBase.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "Delegates/Delegate.h"

class AFortAthenaAIBotController;
class AFortAthenaVehicle;
class AFortPlayerControllerAthena;
class AFortSafeZoneIndicator;
class APlayerState;
class UFortServerBotManagerAthena;
class UFortWeaponItemDefinition;
class UDataTable;
enum class EDeathCause : uint8;

class UAthenaAISettings;

class AFortGameModeAthena : public AFortGamePvPBase
{
public:
    static UClass* StaticClass();

    void InitGameState();

    void FinishWorldInitialization();

    void CreateAIDirector();

    bool ReadyToStartMatch_Implementation();

    int32 CountReadyPlayers();

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

    void PlacePlayerOnTeam(AController* Player);

    void AddToAlivePlayers(AFortPlayerControllerAthena* PlayerController);

    void RemoveFromAlivePlayers(AFortPlayerControllerAthena* PC, APlayerState* RemovalInstigator, APawn* FinisherPawn, UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause, bool bIsTeamSwitching);

    void OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions);

    void OnPlaylistDataLoaded();

    void OverrideAISettings(UAthenaAISettings* InAISettings);

    UAthenaAISettings*& GetAISettings();

    UFortServerBotManagerAthena** GetServerBotManager();

    TSubclassOf<UFortServerBotManagerAthena> GetServerBotManagerClass() const;

    int32& GetWarmupRequiredPlayerCount();

    TArray<AFortPlayerControllerAthena*>& GetAlivePlayers();

    TArray<AFortAthenaAIBotController*>& GetAliveBots();

    bool GetDisableGCOnServerDuringMatch() const;

    FName RedirectLootTierGroup(FName LootTierGroup) const;

    void PerformAsyncLoadForGameFeatureLootData();

    TArray<TSoftObjectPtr<UDataTable>>& GetGameFeatureLootTierData();

    TArray<TSoftObjectPtr<UDataTable>>& GetGameFeatureLootPackageData();

    FSimpleMulticastDelegate* GetOnPlaylistLootTablesAppliedDelegate();

    TSubclassOf<AActor> GetVehicleClassOverride(TSubclassOf<AActor> DefaultVehicleClass) const;

    bool ShouldSpawnVehicle(TSubclassOf<AFortAthenaVehicle> VehicleClass);

    bool IsMapInfoInitialized() const;

    void StartNewSafeZonePhase(int32 NewSafeZonePhase);

    void SetSafeZoneIndicatorShrinkTimes();

    int32 GetSafeZonePhase() const;

    AFortSafeZoneIndicator* GetSafeZoneIndicator() const;

    TArray<FVector>& GetSafeZoneLocations();
};
