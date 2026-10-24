#pragma once

#include "Building/BuildingActor.h"
#include "Math/Vector.h"

class AFortPawn;
struct FFrame;
class AFortPickup;
class UFortWorldItemDefinition;

class AFortAthenaSupplyDrop : public ABuildingActor
{
public:
    AFortPickup* SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const TGameValue<FVector>& GamePosition, const TGameValue<FVector>& GameDirection);

    void execSpawnPickup(FFrame& Stack, void* const Z_Param__Result);
};
