#pragma once

#include "FortGamePvPBase.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "Delegates/Delegate.h"

class AFortAthenaAIBotController;
class AFortAthenaVehicle;
class AFortPlayerControllerAthena;
class AFortPlayerStateAthena;
class AFortSafeZoneIndicator;
class AFortInventory;
class AFortLiveSpectatorController;
class AFortPawn;
class APlayerState;
class UFortServerBotManagerAthena;
class UFortWeaponItemDefinition;
class UDataTable;
enum class EDeathCause : uint8;

class UAthenaAISettings;

struct FFortCustomRepNodeClassMapping
{
    UClass* NodeClass;
    TArray<UClass*> ClassesToRoute;
    bool bShouldDisableSquadNodes;
    bool bShouldDisableHighFrequencyPawnRouting;
};

class AFortGameModeAthena : public AFortGamePvPBase
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, AFortGameModeAthena*>& GetNotifyPlaylistDataLoaded();

    static TMulticastDelegate<void, AFortLiveSpectatorController*>& GetNotifyLiveSpectatorJoined();

    static TMulticastDelegate<void, AFortPlayerStateAthena*, uint8>& GetNotifyPlayerStateSquadAssignment();

    static TMulticastDelegate<void, TSubclassOf<AFortPawn>>& GetNotifyNewAIClass();

    static TMulticastDelegate<void, AActor*, AFortInventory*>& GetNotifyAddReplicatedNonPlayerInventory();

    static TMulticastDelegate<void, AActor*, AFortInventory*>& GetNotifyRemoveReplicatedNonPlayerInventory();

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

    TArray<FFortCustomRepNodeClassMapping>& GetGlobalRepGraphNodeClassesToAdd();

    TSubclassOf<AActor> GetVehicleClassOverride(TSubclassOf<AActor> DefaultVehicleClass) const;

    bool ShouldSpawnVehicle(TSubclassOf<AFortAthenaVehicle> VehicleClass);

    bool IsMapInfoInitialized() const;

    void StartNewSafeZonePhase(int32 NewSafeZonePhase);

    void SetSafeZoneIndicatorShrinkTimes();

    int32 GetSafeZonePhase() const;

    void SetSafeZonePhase(int32 InSafeZonePhase);

    AFortSafeZoneIndicator* GetSafeZoneIndicator() const;

    TArray<FVector>& GetSafeZoneLocations();
};
