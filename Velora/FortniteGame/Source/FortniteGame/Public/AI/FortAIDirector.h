#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"

class UFortAIEncounterInfo;
class UScriptStruct;

struct FFortAIEncounterSpawnGroupCap
{
    FFortAIEncounterSpawnGroupCap();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FCurveTableRowHandle& GetMinSpawnGroupNumberCap();

    FCurveTableRowHandle& GetMaxSpawnGroupNumberCap();
};

template<>
class TArray<FFortAIEncounterSpawnGroupCap, FDefaultAllocator> : public TRuntimeSizedArray<FFortAIEncounterSpawnGroupCap>
{
};

struct FFortAIEncounterSpawnGroupCapsCategory
{
    FFortAIEncounterSpawnGroupCapsCategory();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FGameplayTagQuery& GetTagQuery();

    FCurveTableRowHandle& GetInitialSpawnGroupAvailabilityDelaySeconds();

    FCurveTableRowHandle& GetSpawnGroupAvailabilityDelaySeconds();

    TArray<FFortAIEncounterSpawnGroupCap>& GetSpawnGroupCapsPerPlayerCount();

    float& GetInitialSpawnGroupAvailabilityTime();
};

template<>
class TArray<FFortAIEncounterSpawnGroupCapsCategory, FDefaultAllocator> : public TRuntimeSizedArray<FFortAIEncounterSpawnGroupCapsCategory>
{
};

struct FFortAIEncounterSpawnGroupCapsProfile
{
    FFortAIEncounterSpawnGroupCapsProfile();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FGameplayTagContainer& GetEncounterTypeTags();

    TArray<FFortAIEncounterSpawnGroupCapsCategory>& GetPopulationCategories();
};

template<>
class TArray<FFortAIEncounterSpawnGroupCapsProfile, FDefaultAllocator> : public TRuntimeSizedArray<FFortAIEncounterSpawnGroupCapsProfile>
{
};

struct FFortAIEncounterSpawnPointsProfile
{
    FFortAIEncounterSpawnPointsProfile();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FGameplayTagContainer& GetEncounterTypeTags();

    TArray<FCurveTableRowHandle>& GetMaxSpawnPointsPerPlayerCount();

    TArray<FCurveTableRowHandle>& GetMinSpawnPointsPerPlayerCount();
};

template<>
class TArray<FFortAIEncounterSpawnPointsProfile, FDefaultAllocator> : public TRuntimeSizedArray<FFortAIEncounterSpawnPointsProfile>
{
};

class AFortAIDirector : public AActor
{
public:
    static UClass* StaticClass();

    TArray<FFortAIEncounterSpawnGroupCapsProfile>& GetEncounterSpawnGroupCapSettings();

    TArray<FFortAIEncounterSpawnPointsProfile>& GetEncounterSpawnPointsSettings();

    TArray<FDataTableRowHandle>& GetBaseLootDropData();

    TSubclassOf<UFortAIEncounterInfo>& GetDummyDebugEncounter();

    TSubclassOf<UFortAIEncounterInfo>& GetBaseEncounterClass();
};
