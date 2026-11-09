#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AFortAthenaAircraft;
class AFortPlayerPawn;
class AFortPlayerPawnAthena;
class AFortSafeZoneIndicator;
class APlayerState;
class UObject;
struct FFrame;
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

    void InitializeSafeZoneLocations();

    void TryToProceedToAircraftPhase();

    bool StartAircraftPhase(bool bSkippedWarmup);

    void SetupAircraft(AFortAthenaAircraft* Aircraft);

    void execOnAircraftEnteredDropZone(FFrame& Stack, void* const Z_Param__Result);

    void OnAircraftEnteredDropZone(AFortAthenaAircraft* FortAthenaAircraft);

    void execOnAircraftExitedDropZone(FFrame& Stack, void* const Z_Param__Result);

    void OnAircraftExitedDropZone(AFortAthenaAircraft* FortAthenaAircraft);

    void execOnAircraftFlightEnded(FFrame& Stack, void* const Z_Param__Result);

    void OnAircraftFlightEnded(AFortAthenaAircraft* FortAthenaAircraft);

    AFortAthenaAircraft* GetAircraft(int32 AircraftIndex) const;

    int32 GetAircraftIndex(APlayerState* PlayerState);

    int32 GetAircraftIndexFromTeam(uint8 TeamIndex);

    void SetAircraftLock(bool bLock);

    static bool HasSafeZoneAlreadyStarted(EAthenaGamePhase InGamePhase, ESafeZoneStartUp SafeZoneStartUp);

    bool StartSafeZones(bool bForce, bool bStartSafeZonesPhase);

    bool StartSafeZonesPhase(bool bForce);

    void SpawnInitialSafeZone();

    void UpdateSafeZonesPhase();

    bool IsSafeZoneTimerPaused() const;

    void HandlePawnPossessed(AFortPlayerPawnAthena* Pawn);

    void SafeZoneInsideChecks();

    void SafeZoneSetPlayerInside(AFortPlayerPawn* Pawn, int32 StormDamageLevel, bool bIsInsideSafeZone, bool bUseStormDamageLevelForShieldDamage);

    bool IsInCurrentSafeZone(const FVector& TestLocation, bool bClassicSafeZoneOnly);

    void execSetStormDamagePaused(FFrame& Stack, void* const Z_Param__Result);

    void SetStormDamagePaused(bool bPause);

    bool& GetStormDamagePaused();

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

    int32& GetNumAircraftsExitedDropZone();

    float& GetAircraftStartTime();

    double& GetAircraftRealStartTime();
};
