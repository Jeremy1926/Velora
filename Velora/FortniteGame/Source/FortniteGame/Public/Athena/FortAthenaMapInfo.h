#pragma once

#include "GameFramework/Actor.h"

class AFortAthenaSupplyDrop;
struct FScalableFloat;

class AFortAthenaMapInfo : public AActor
{
public:
    static UClass* StaticClass();

    void SpawnLlamas();

    FVector PickSupplyDropLocation(const FVector& Center, float Radius) const;

    AFortAthenaSupplyDrop* SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass) const;

    UClass* GetLlamaClass() const;

    const FScalableFloat& GetLlamaQuantityMin() const;

    const FScalableFloat& GetLlamaQuantityMax() const;

    const FScalableFloat* GetSafeZoneRadius() const;
};
