#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayEffectTypes.h"
#include "UObject/SoftObjectPtr.h"

class UFortAthenaLivingWorldConfigData : public UDataAsset
{
public:
    static UClass* StaticClass();

    TSoftObjectPtr<UDataTable>& GetCategoryTable();

    FScalableFloat& GetMaxActorCount();

    FScalableFloat& GetMaxEventSpawnPerTick();

    FScalableFloat& GetMinimumEventGenerationInterval();

    FScalableFloat* GetMaxActorDensity();

    FScalableFloat* GetDensityComputationDistance();
};
