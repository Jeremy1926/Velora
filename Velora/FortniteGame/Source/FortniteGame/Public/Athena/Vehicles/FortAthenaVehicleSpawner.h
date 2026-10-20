#pragma once

#include "GameFramework/Actor.h"

class AFortAthenaVehicleSpawner : public AActor
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void SpawnVehicle();

    UClass* GetVehicleClass() const;
};
