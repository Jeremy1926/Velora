#pragma once

#include "UObject/Object.h"
#include "Math/TransformVectorized.h"

class UFortAthenaAISpawnerDataComponentList;

class UAthenaAISpawner : public UObject
{
public:
    int32 RequestSpawn(UFortAthenaAISpawnerDataComponentList* AISpawnerComponentList, const FTransform& SpawnTransform);
};
