#pragma once

#include "Building/BuildingActor.h"
#include "UObject/NameTypes.h"

class UEnvQuery;

class ABGAConsumableSpawner : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void SpawnConsumables();

    void SpawnConsumablesAtLocations(TArray<FVector> Locations);

    static void TickPendingSpawns();

    FName GetSpawnLootTierGroup() const;

    UEnvQuery* GetQueryTemplate() const;
};
