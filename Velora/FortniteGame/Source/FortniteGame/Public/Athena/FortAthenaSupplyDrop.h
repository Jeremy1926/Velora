#pragma once

#include "Building/BuildingActor.h"

class AFortPawn;
struct FFrame;
class AFortPickup;
class UFortWorldItemDefinition;

class AFortAthenaSupplyDrop : public ABuildingActor
{
public:
    AFortPickup* SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const FVector& Position, const FVector& Direction);

    void execSpawnPickup(FFrame& Stack, void* const Z_Param__Result);
};
