#pragma once

#include "Engine/DataAsset.h"
#include "UObject/Class.h"
#include "Containers/Array.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "UObject/SoftObjectPtr.h"

class UEnvQuery;

struct FFortAthenaLivingWorldEventDataActorSpawnDescription
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    TSoftClassPtr<UClass>& GetActorClass();

    TSoftClassPtr<UClass>& GetSpawnerData();

    FGameplayTagQuery& GetSpawnerDataTagQuery();

    bool GetbSpawnAroundDefaultPoint();

    UEnvQuery* GetSpawnAroundEnvironmentQuery();
};

class UFortAthenaLivingWorldEventData : public UDataAsset
{
public:
    static UClass* StaticClass();

    FGameplayTagQuery& GetProviderFiltersTagQuery();

    FScalableFloat* GetMinActorCountToSpawn();

    FScalableFloat* GetMaxActorCountToSpawn();

    TRuntimeSizedArray<FFortAthenaLivingWorldEventDataActorSpawnDescription>& GetActorDescriptions();
};
