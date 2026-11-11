#pragma once

#include "GameFramework/Actor.h"
#include "Athena/FortAthenaAircraft.h"
#include "Containers/Array.h"
#include "UObject/AssetPtr.h"

class AFortAthenaSupplyDrop;
class AFortGameStateAthena;
class UFortGameStateComponent_BattleRoyaleGamePhaseLogic;
class UScriptStruct;
struct FScalableFloat;

struct FVehicleClassDetails
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    TAssetPtr<UClass>& GetVehicleClass();

    const FScalableFloat& GetVehicleMinSpawnPercent();

    const FScalableFloat& GetVehicleMaxSpawnPercent();
};

template<>
class TArray<FVehicleClassDetails, FDefaultAllocator> : public TRuntimeSizedArray<FVehicleClassDetails>
{
};

class AFortAthenaMapInfo : public AActor
{
public:
    static UClass* StaticClass();

    TArray<FVehicleClassDetails>& GetVehicleClassDetails();

    void SpawnLlamas();

    bool IsMapInfoValidForAircraft() const;

    void InitializeFlightPath(AFortGameStateAthena* GameState, UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic, bool bForceDropCenterToFirstSafeZone, float PathMidpointSelectionRadiusMin, float PathMidpointSelectionRadiusMax, float LastStepPushCenterLine_Magnitude);

    FVector PickSupplyDropLocation(const FVector& Center, float Radius, bool bDisableSupplyDropsSpawningOverWater, float TraceStartZ, float TraceEndZ) const;

    AFortAthenaSupplyDrop* SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const;

    AFortAthenaSupplyDrop* SpawnSupplyDropWithinCircle(const TGameValue<FVector>& Center, float Radius, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const;

    AFortAthenaSupplyDrop* SpawnSupplyDropInSafeZone(int32 GamePhaseIndex, int32 SafeZoneIndex, const UObject* SupplyDropInfo, const TGameValue<FVector>& SafeZoneLocation) const;

    UClass* GetLlamaClass() const;

    UClass* GetAircraftClass() const;

    TArray<FAircraftFlightInfo>& GetFlightInfos();

    const FScalableFloat& GetLlamaQuantityMin() const;

    const FScalableFloat& GetLlamaQuantityMax() const;

    const FScalableFloat* GetVendingMachineRarityCount() const;

    const FScalableFloat* GetWeaponUpgradeMachineSpawnChance() const;

    const FScalableFloat* GetSupplyDropMinPlacementHeight() const;

    const FScalableFloat* GetSupplyDropMaxPlacementHeight() const;

    const FScalableFloat* GetSafeZoneRadius() const;

    const FScalableFloat& GetSafeZoneStartingRadius() const;

    const FScalableFloat& GetSafeZoneStartDelay() const;

    int32 GetSafeZoneCount() const;

    void GenerateSafeZoneLocations(TArray<uint8>& OutLocations, const FVector& MapCenter, int32 BaseSafeZoneIndex);

    const uint8* GetSafeZoneDefinition() const;
};
