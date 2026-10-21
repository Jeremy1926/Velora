#pragma once

#include "Engine/DataAsset.h"
#include "UObject/Class.h"
#include "Containers/Array.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

struct FFortAthenaLivingWorldEventDataActorSpawnDescription
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    TSoftClassPtr<UClass>& GetSpawnerData();

    FGameplayTagQuery& GetSpawnerDataTagQuery();
};

class UFortAthenaLivingWorldEventData : public UDataAsset
{
public:
    static UClass* StaticClass();

    FGameplayTagQuery& GetProviderFiltersTagQuery();

    TRuntimeSizedArray<FFortAthenaLivingWorldEventDataActorSpawnDescription>& GetActorDescriptions();
};
