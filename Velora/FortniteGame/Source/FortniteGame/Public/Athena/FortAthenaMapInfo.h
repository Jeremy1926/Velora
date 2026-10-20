#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "UObject/AssetPtr.h"

class AFortAthenaSupplyDrop;
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

    FVector PickSupplyDropLocation(const FVector& Center, float Radius, bool bDisableSupplyDropsSpawningOverWater, float TraceStartZ, float TraceEndZ) const;

    AFortAthenaSupplyDrop* SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const;

    UClass* GetLlamaClass() const;

    const FScalableFloat& GetLlamaQuantityMin() const;

    const FScalableFloat& GetLlamaQuantityMax() const;

    const FScalableFloat* GetVendingMachineRarityCount() const;

    const FScalableFloat* GetSupplyDropMinPlacementHeight() const;

    const FScalableFloat* GetSupplyDropMaxPlacementHeight() const;

    const FScalableFloat* GetSafeZoneRadius() const;
};
