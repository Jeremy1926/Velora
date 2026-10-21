#pragma once

#include "UObject/Object.h"
#include "UObject/ScriptDelegates.h"
#include "Math/TransformVectorized.h"

class UFortAthenaAISpawnerDataComponentList;

struct FOnPawnSpawnedDynamicEvent : public FMulticastScriptDelegate
{
    void ProcessMulticastDelegate(void* Parameters) const;
};

class UAthenaAISpawner : public UObject
{
public:
    int32 RequestSpawn(UFortAthenaAISpawnerDataComponentList* AISpawnerComponentList, const FTransform& SpawnTransform);
};
