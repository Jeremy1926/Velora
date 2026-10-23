#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class APawn;

class UFortAthenaAISpawnerDataComponent_Vehicle : public UObject
{
public:
    void PostOnSpawned(APawn* PawnAI);

    UClass* GetVehicleToSpawnAndEnter() const;

    bool GetbSetInfiniteFuel() const;

    const TArray<UClass*>* GetVehicleSkillSets() const;
};
