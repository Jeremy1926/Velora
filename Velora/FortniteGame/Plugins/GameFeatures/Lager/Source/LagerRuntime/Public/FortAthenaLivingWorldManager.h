#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class AFortGameStateAthena;
class UDataTable;
class UFortAthenaLivingWorldConfigData;
class UFortAthenaLivingWorldEventData;
class UWorld;
struct FFortAthenaLivingWorldCategory;
struct FFortAthenaLivingWorldEvent;
struct FFortAthenaLivingWorldEventDataActorSpawnDescription;
struct FFortAthenaLivingWorldPointProviderFilterRules;

class UFortAthenaLivingWorldManager : public UActorComponent
{
public:
    struct FLivingWorldEventInstance
    {
        FFortAthenaLivingWorldCategory* Category = nullptr;
        FFortAthenaLivingWorldEvent* Event = nullptr;
        TWeakObjectPtr<AActor> PointProvider;
        TArray<int32> PendingRequestIDs;
        TArray<TWeakObjectPtr<AActor>> SpawnedActors;
        float RespawnTime = 0.0f;
        int32 ActorCount = 0;
    };

    struct FLivingWorldEventRuntimeData
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        const FFortAthenaLivingWorldEvent* Event = nullptr;
        float ActivationTime = -1.0f;
        int32 SpawnedCount = 0;
    };

    static UClass* StaticClass();

    static int32 GetEventInstancesOffset();

    static void LivingWorldManagerRegisterPointProvider(AActor* PointProvider);

    bool IsLivingWorldEnabled();

    void TickComponent(float DeltaTime);

    void GenerateEvents();

    void OnActorSpawned(AActor* SpawnedActor, int32 RequestID);

    void ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight);

    int32 GetEventBudget(FFortAthenaLivingWorldEvent* Event);

    int32 GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event);

    AActor* GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter);

    bool RequestSpawnToAISpawnerSystem(FFortAthenaLivingWorldCategory* Category, FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData);

    UClass* GetSpawnerDataClass(FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription);

    FLivingWorldEventRuntimeData& FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event);

    TSoftObjectPtr<UFortAthenaLivingWorldConfigData>& GetLagerConfig();

    FScalableFloat& GetLagerEnabled();

    TArray<AActor*>& GetPointProviders();

    TArray<UClass*>& GetLoadedSpawnerDataClass();

    AFortGameStateAthena*& GetCachedGameState();

    TArray<AActor*>& GetRuntimePointProviderList();

    TArray<FLivingWorldEventInstance>& GetEventInstances();

    float& GetNextEventGenerationTime();

    FGameplayTagContainer& GetPlaylistContextTags();

    UWorld** GetCachedWorld();

private:
    static TArray<FLivingWorldEventRuntimeData> EventRuntimeData;
};
