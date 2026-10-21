#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"

class AActor;
class UFortVehicleItemDefinition;

class UFortDagwoodVehicleManager : public UActorComponent
{
public:
    static UClass* StaticClass();

    void OnVehicleSpawnsUpdated(UFortVehicleItemDefinition* VehicleItemDef, TArray<AActor*>& SpawnedVehicles, int32 NewVehicleCount);
};
