#pragma once

#include "Building/BuildingActor.h"

class AFortPawn;
class AFortPickup;
class UFortWorldItemDefinition;

class AFortAthenaSupplyDrop : public ABuildingActor
{
public:
    AFortPickup* SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const FVector& Position, const FVector& Direction);
};
