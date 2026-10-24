#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Queue.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/ScriptInterface.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class AFortGameStateAthena;
class UDataTable;
class UFortAthenaLivingWorldConfigData;
class UFortAthenaLivingWorldEncounterInstance;
class UFortAthenaLivingWorldEventData;
class UWorld;
struct FFortAthenaLivingWorldCategory;
struct FFortAthenaLivingWorldDensityGrid;
struct FFortAthenaLivingWorldEvent;
struct FFortAthenaLivingWorldEventDespawnCondition;
struct FFortAthenaLivingWorldEventDataActorSpawnDescription;
struct FFortAthenaLivingWorldPlayerTracker;
struct FFortAthenaLivingWorldPointProviderFilterRules;

class UFortAthenaLivingWorldManager : public UActorComponent
{
public:
    struct FLivingWorldPendingSpawnRequest
    {
        int32 RequestID = INDEX_NONE;
        TWeakObjectPtr<AActor> PointProvider;
        FVector Location = FVector(ForceInit);
        float DensityValue = 0.0f;
        float DensityRange = -1.0f;
    };

    struct FLivingWorldEventInstance
    {
        static int32 GetTypeSize();

        static int32 GetRespawnTimeOffset();

        static int32 GetSpawnedActorsOffset();

        static int32 GetSpawnedActorPointProvidersOffset();

        static int32 GetPendingSpawnRequestsOffset();

        static int32 GetEncounterInstanceOffset();

        ~FLivingWorldEventInstance();

        TArray<TWeakObjectPtr<AActor>>& GetSpawnedActors();

        TArray<FScriptInterface>* GetSpawnedActorPointProviders();

        TArray<FLivingWorldPendingSpawnRequest>& GetPendingSpawnRequests();

        UFortAthenaLivingWorldEncounterInstance*& GetEncounterInstance();

        float& GetRespawnTime();

        int32& GetActorCount();

        FFortAthenaLivingWorldCategory* Category;
        FFortAthenaLivingWorldEvent* Event;
        TWeakObjectPtr<UFortAthenaLivingWorldEventData> EventData;
    };

    struct FLivingWorldEventRequestData
    {
        FFortAthenaLivingWorldCategory* Category;
        FFortAthenaLivingWorldEvent* Event;
        TWeakObjectPtr<UFortAthenaLivingWorldEventData> EventData;
        UFortAthenaLivingWorldEncounterInstance* EncounterInstance;
        int32 ActorCountToSpawn;
    };

    struct FLivingWorldPendingActorSpawnEventRequest
    {
        struct FRequestInfo
        {
            FRequestInfo();

            void SetLocation(const FVector& InLocation);

            FScriptInterface& GetPointProvider();

            int32& GetRequestID();

            int32& GetPreRegistrationHandle();

        private:
            template<typename LocationType>
            struct TLayout
            {
                LocationType Location;
                TWeakObjectPtr<AActor> Actor;
                FScriptInterface PointProvider;
                int32 RequestID;
                int32 PreRegistrationHandle;
            };

            alignas(void*) uint8 Data[0x50];
        };

        FLivingWorldEventRequestData RequestData;
        TArray<FRequestInfo> RequestInfos;
        TDelegate<void(const TArray<AActor*>&, bool)> OnSpawned;
    };

    struct FLivingWorldEventRequest
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        FFortAthenaLivingWorldCategory* Category = nullptr;
        FFortAthenaLivingWorldEvent* Event = nullptr;
        TWeakObjectPtr<UFortAthenaLivingWorldEventData> EventData;
        TWeakObjectPtr<AActor> PointProvider;
        TWeakObjectPtr<UFortAthenaLivingWorldEncounterInstance> EncounterInstance;
        int32 ActorCountToSpawn = 0;
    };

    struct FLivingWorldPendingEQSSpawn
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        TWeakObjectPtr<AActor> PointProvider;
        TWeakObjectPtr<UClass> SpawnerDataClass;
        TWeakObjectPtr<UClass> ActorClass;
        TWeakObjectPtr<UObject> SpawnAroundEnvironmentQuery;
        FVector DefaultPosition = FVector(ForceInit);
        FRotator DefaultRotation{ 0.0f, 0.0f, 0.0f };
        int32 RequestID = INDEX_NONE;
        int32 QueryID = INDEX_NONE;
        float QueryStartTime = 0.0f;
    };

    struct FLivingWorldEventRuntimeData
    {
        FLivingWorldEventRuntimeData();

        static int32 GetSpawnedCountOffset();

        static int32 GetTotalPointProviderWeightOffset();

        static int32 GetDensityComputationRangeOffset();

        float& GetActivationTime();

        int32& GetSpawnedCount();

        float& GetTotalPointProviderWeight();

        float& GetDensityComputationRange();

    private:
        alignas(float) uint8 Data[0x14];
    };

    struct FLivingWorldActorUpdateInfo
    {
        TWeakObjectPtr<AActor> Actor;
        FFortAthenaLivingWorldEvent* Event = nullptr;
        float IsDespawnCandidateSinceTimestamp = -1.0f;
        TArray<FGameplayTag, TInlineAllocator<1>> TagDensityToUpdate;
    };

    struct FLivingWorldCategoryRuntimeData
    {
        UDataTable* CurrentEventTable = nullptr;
        int32 GameSpawnedCount = 0;
    };

    struct FLivingWorldEventRuntimeDataEntry
    {
        TWeakObjectPtr<UFortAthenaLivingWorldManager> LivingWorldManager;
        const FFortAthenaLivingWorldEvent* Event = nullptr;
        FLivingWorldEventRuntimeData RuntimeData;
    };

    static UClass* StaticClass();

    static int32 GetEventInstancesOffset();

    static int32 GetPendingActorSpawnEventRequestsOffset();

    static void LivingWorldManagerRegisterPointProvider(AActor* PointProvider);

    static void LivingWorldManagerUnregisterPointProvider(AActor* PointProvider);

    bool IsLivingWorldEnabled();

    void TickComponent(float DeltaTime);

    void GenerateEvents();

    void GenerateEventRequests(UFortAthenaLivingWorldEncounterInstance* EncounterInstance, FFortAthenaLivingWorldCategory* Category, UDataTable* EventTable);

    void UpdateEncounterInstances();

    int32 ComputeCategoryRemainingBudget(FFortAthenaLivingWorldCategory* Category);

    void ProcessEventRequests();

    void UpdateActorsInfo();

    void DespawnActors();

    bool IsActorCandidateForDespawn(AActor* Actor, FFortAthenaLivingWorldEventDespawnCondition* DespawnCondition);

    void ProcessEQSRequests();

    void OnEQSRequestFinished(TSharedPtr<FEnvQueryResult> Result);

    void SpawnFromEQSRequest(const FLivingWorldPendingEQSSpawn& PendingEQSSpawn, const FVector* Location);

    void OnActorSpawned(AActor* SpawnedActor, int32 RequestID);

    void OnActorSpawned(FLivingWorldEventInstance& EventInstance, AActor* SpawnedActor, AActor* PointProvider, const FVector& PositionFromProvider, float DensityValue, float DensityRange);

    void ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight, UFortAthenaLivingWorldEncounterInstance* EncounterInstance);

    bool IsEventActive(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData);

    int32 GetEventBudget(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEncounterInstance* EncounterInstance);

    int32 GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event);

    int32 GetTotalEncounterInstanceActorCount(UFortAthenaLivingWorldEncounterInstance* EncounterInstance);

    AActor* GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, UFortAthenaLivingWorldEncounterInstance* EncounterInstance, FLivingWorldEventRuntimeData& RuntimeData);

    bool RequestSpawnToAISpawnerSystem(const FLivingWorldEventRequest& EventRequest);

    UClass* GetSpawnerDataClass(FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription);

    FLivingWorldEventRuntimeData& FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEncounterInstance* EncounterInstance);

    FLivingWorldCategoryRuntimeData* FindOrCreateCategoryRuntimeData(const FFortAthenaLivingWorldCategory* Category);

    UFortAthenaLivingWorldConfigData* GetConfig();

    void AddPointProvider(AActor* PointProvider);

    void RemovePointProvider(AActor* PointProvider);

    TSoftObjectPtr<UFortAthenaLivingWorldConfigData>& GetLagerConfig();

    FScalableFloat& GetLagerEnabled();

    bool HasPointProviderInterfaces();

    TArray<AActor*>& GetPointProviders();

    TArray<FScriptInterface>& GetPointProviderInterfaces();

    TArray<UClass*>& GetLoadedSpawnerDataClass();

    AFortGameStateAthena* GetCachedGameState();

    void SetCachedGameState(AFortGameStateAthena* GameState);

    FFortAthenaLivingWorldPlayerTracker* GetPlayerTracker();

    TArray<AActor*>& GetRuntimePointProviderList();

    TArray<UFortAthenaLivingWorldEncounterInstance*>& GetRunningEncounterInstances();

    TRuntimeSizedArray<FLivingWorldEventInstance>& GetEventInstances();

    TArray<FLivingWorldPendingActorSpawnEventRequest>& GetPendingActorSpawnEventRequests();

    TQueue<FLivingWorldActorUpdateInfo>* GetActorInfoUpdateQueue();

    float& GetNextEventGenerationTime();

    FGameplayTagContainer& GetPlaylistContextTags();

    UWorld** GetCachedWorld();

    AActor* GetEQSHelper();

    FFortAthenaLivingWorldDensityGrid* GetActorDensityGrid();

    FFortAthenaLivingWorldDensityGrid* GetTagDensityGrid(const FGameplayTag& Tag);

    TArray<AActor*>* GetSpawnedPawns();

    float GetActorDensity(const FVector& Location, float Radius);

    int32 GetActorDensityBudget(const FVector& Location, float Radius);

    float GetMaxActorDensity();

    void ClusterPoints(const TArray<FVector>& Points, TArray<FVector>& ClusteredPoints);

private:
    static TArray<FLivingWorldEventRuntimeDataEntry> EventRuntimeData;

    static TArray<FLivingWorldEventRequest> PendingEventRequests;

    static TArray<FLivingWorldPendingEQSSpawn> PendingEQSSpawns;

    static TArray<FLivingWorldActorUpdateInfo> ActorToDespawnQueue;

    static int32 NextEQSSpawnRequestID;
};
