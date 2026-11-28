#pragma once

#include "Building/BuildingActor.h"
#include "UObject/NameTypes.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;

class ABGAConsumableSpawner : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void SpawnConsumables();

    void SpawnConsumablesAtLocations(TArray<FVector> Locations);

    static void OnSpawnQueryFinished(UEnvQueryInstanceBlueprintWrapper* QueryInstance);

    static void TickPendingSpawns();

    FName GetSpawnLootTierGroup() const;

    UEnvQuery* GetQueryTemplate() const;

    TArray<FAIDynamicParam>& GetQueryConfig();
};
