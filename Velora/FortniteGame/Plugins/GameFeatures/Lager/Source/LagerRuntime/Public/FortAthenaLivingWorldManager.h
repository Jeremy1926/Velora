#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Math/Vector.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/ScriptInterface.h"
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
    struct FLivingWorldPendingSpawnRequest
    {
        int32 RequestID = INDEX_NONE;
        TWeakObjectPtr<AActor> PointProvider;
    };

    struct FLivingWorldEventInstance
    {
        static int32 GetTypeSize();

        static int32 GetRespawnTimeOffset();

        ~FLivingWorldEventInstance();

        TArray<TWeakObjectPtr<AActor>>& GetSpawnedActors();

        float& GetRespawnTime();

        int32& GetActorCount();

        FFortAthenaLivingWorldCategory* Category;
        FFortAthenaLivingWorldEvent* Event;
        TWeakObjectPtr<UFortAthenaLivingWorldEventData> EventData;
        TArray<FLivingWorldPendingSpawnRequest> PendingSpawnRequests;
    };

    struct FLivingWorldEventRequest
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        FFortAthenaLivingWorldCategory* Category = nullptr;
        FFortAthenaLivingWorldEvent* Event = nullptr;
        TWeakObjectPtr<UFortAthenaLivingWorldEventData> EventData;
        TWeakObjectPtr<AActor> PointProvider;
    };

    struct FLivingWorldEventRuntimeData
    {
        float ActivationTime = -1.0f;
        int32 SpawnedCount = 0;
    };

    struct FLivingWorldEventRuntimeDataEntry
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        const FFortAthenaLivingWorldEvent* Event = nullptr;
        FLivingWorldEventRuntimeData RuntimeData;
    };

    static UClass* StaticClass();

    static int32 GetEventInstancesOffset();

    static void LivingWorldManagerRegisterPointProvider(AActor* PointProvider);

    static void LivingWorldManagerUnregisterPointProvider(AActor* PointProvider);

    bool IsLivingWorldEnabled();

    void TickComponent(float DeltaTime);

    void GenerateEvents();

    void ProcessEventRequests();

    void OnActorSpawned(AActor* SpawnedActor, int32 RequestID);

    void OnActorSpawned(FLivingWorldEventInstance& EventInstance, AActor* SpawnedActor, AActor* PointProvider);

    void ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight);

    bool IsEventActive(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData);

    int32 GetEventBudget(FFortAthenaLivingWorldEvent* Event);

    int32 GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event);

    AActor* GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter);

    bool RequestSpawnToAISpawnerSystem(const FLivingWorldEventRequest& EventRequest);

    UClass* GetSpawnerDataClass(FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription);

    FLivingWorldEventRuntimeData& FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event);

    UFortAthenaLivingWorldConfigData* GetConfig();

    void AddPointProvider(AActor* PointProvider);

    void RemovePointProvider(AActor* PointProvider);

    TSoftObjectPtr<UFortAthenaLivingWorldConfigData>& GetLagerConfig();

    FScalableFloat& GetLagerEnabled();

    bool HasPointProviderInterfaces();

    TArray<AActor*>& GetPointProviders();

    TArray<FScriptInterface>& GetPointProviderInterfaces();

    TArray<UClass*>& GetLoadedSpawnerDataClass();

    AFortGameStateAthena*& GetCachedGameState();

    TArray<AActor*>& GetRuntimePointProviderList();

    TRuntimeSizedArray<FLivingWorldEventInstance>& GetEventInstances();

    float& GetNextEventGenerationTime();

    FGameplayTagContainer& GetPlaylistContextTags();

    UWorld** GetCachedWorld();

    TArray<AActor*>* GetSpawnedPawns();

    float GetActorDensity(const FVector& Location, float Radius);

    int32 GetActorDensityBudget(const FVector& Location, float Radius);

private:
    static TArray<FLivingWorldEventRuntimeDataEntry> EventRuntimeData;

    static TArray<FLivingWorldEventRequest> PendingEventRequests;
};
