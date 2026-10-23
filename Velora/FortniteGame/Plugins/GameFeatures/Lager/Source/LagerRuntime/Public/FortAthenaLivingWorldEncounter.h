#pragma once

#include "Containers/Array.h"
#include "Engine/DataAsset.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "UObject/ScriptInterface.h"
#include "UObject/SoftObjectPtr.h"

class UDataTable;
struct FFrame;
struct FFortAthenaLivingWorldCategory;

struct FFortAthenaLivingWorldEncounterStage
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    TSoftObjectPtr<UDataTable>& GetEventTable();

    FScalableFloat& GetMaximumConcurrentNumberOfAI();
};

class UFortAthenaLivingWorldEncounter : public UDataAsset
{
public:
    static UClass* StaticClass();

    FScalableFloat& GetIntervalBetweenEventGeneration();

    TRuntimeSizedArray<FFortAthenaLivingWorldEncounterStage>& GetStages();
};

class UFortAthenaLivingWorldEncounterInstance : public UObject
{
public:
    static UClass* StaticClass();

    int32 GetTotalActorCount();

    void execGetTotalActorCount(FFrame& Stack, void* const Z_Param__Result);

    TSoftObjectPtr<UFortAthenaLivingWorldEncounter>& GetEncounterDefinition();

    TArray<FScriptInterface>& GetRegisteredPointProviders();

    FFortAthenaLivingWorldCategory*& GetCategory();

    int32& GetCurrentStageIndex();

    float& GetLastGenerationTime();

    bool GetbIsPaused();

    void* GetEventRuntimeDataMap();
};
