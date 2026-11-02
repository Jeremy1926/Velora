#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AFortAthenaAircraft;
class AFortSafeZoneIndicator;
class UObject;
struct FScalableFloat;
struct FVector;

enum class EAthenaGamePhase : uint8;
enum class EAthenaGamePhaseStep : uint8;
enum class ESafeZoneStartUp : uint8;

class UFortGameStateComponent_BattleRoyaleGamePhaseLogic : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Get(const UObject* WorldContextObject);

    void TickComponent(float DeltaTime);

    void HandleMatchHasStarted();

    bool StartWarmupPhase();

    void UpdateWarmupPhase();

    void SpawnAircrafts();

    void TryToProceedToAircraftPhase();

    bool StartAircraftPhase(bool bSkippedWarmup);

    void SetupAircraft(AFortAthenaAircraft* Aircraft);

    static bool HasSafeZoneAlreadyStarted(EAthenaGamePhase InGamePhase, ESafeZoneStartUp SafeZoneStartUp);

    bool StartSafeZones(bool bForce, bool bStartSafeZonesPhase);

    bool StartSafeZonesPhase(bool bForce);

    void SpawnInitialSafeZone();

    void UpdateSafeZonesPhase();

    bool IsSafeZoneTimerPaused() const;

    void SetGamePhase(EAthenaGamePhase InGamePhase);

    void SetGameModeWillSkipAircraft(bool bInGameModeWillSkipAircraft);

    EAthenaGamePhase GetGamePhase() const;

    EAthenaGamePhaseStep GetGamePhaseStep() const;

    float GetSafeZonesStartTime() const;

    void SetSafeZonesStartTime(float InSafeZonesStartTime);

    AFortSafeZoneIndicator*& GetSafeZoneIndicator();

    TArray<FVector>& GetSafeZoneLocations();

    bool GetPlaylistStoppedSafeZonePhases() const;

    const FScalableFloat& GetSafeZoneDamageAttribute() const;

    float& GetWarmupCountdownStartTime();

    float& GetWarmupCountdownEndTime();

    float& GetWarmupCountdownDuration();

    float& GetWarmupEarlyCountdownDuration();

    float GetWarmupEarlyRequiredPlayerPercent() const;

    TArray<TWeakObjectPtr<AFortAthenaAircraft>>& GetAircrafts_GameMode();

    TArray<TWeakObjectPtr<AFortAthenaAircraft>>& GetAircrafts_GameState();

    void SetAircraftIsLocked(bool bInAircraftIsLocked);

    float& GetAircraftStartTime();

    double& GetAircraftRealStartTime();
};
