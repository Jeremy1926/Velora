#pragma once

#include "UObject/Object.h"
#include "Delegates/Delegate.h"
#include "UObject/ScriptDelegates.h"
#include "Math/TransformVectorized.h"

class AActor;
class UFortAthenaAISpawnerDataComponentList;

struct FOnPawnSpawnedDynamicEvent : public FMulticastScriptDelegate
{
    void ProcessMulticastDelegate(void* Parameters) const;
};

class UAthenaAISpawner : public UObject
{
public:
    int32 RequestSpawn(UFortAthenaAISpawnerDataComponentList* AISpawnerComponentList, const FTransform& SpawnTransform, const TDelegate<void(AActor*, int32)>& SpawnedCallback = TDelegate<void(AActor*, int32)>());
};
