#pragma once

#include "GameFramework/Actor.h"

class UFortVehicleItemDefinition;

class AFortAthenaVehicleSpawner : public AActor
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void SpawnVehicle();

    UClass* GetVehicleClass() const;

    bool GetForceSpawnAlways() const;

    UFortVehicleItemDefinition* GetFortVehicleItemDef() const;

    void SetFortVehicleItemDef(UFortVehicleItemDefinition* InFortVehicleItemDef);

    AActor* GetSpawnedVehicle() const;

    bool ShouldSelfSpawnVehicle() const;
};
