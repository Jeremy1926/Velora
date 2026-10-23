#pragma once

#include "Components/ActorComponent.h"
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
struct FFortAthenaLivingWorldEventDataActorSpawnDescription;
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

    struct FLivingWorldTagPreRegistration
    {
        FGameplayTag Tag;
        int32 PreRegistrationHandle;
    };

    struct FLivingWorldPendingActorSpawnEventRequest
    {
        struct FRequestInfo
        {
            FVector Location;
            TWeakObjectPtr<AActor> Actor;
            FScriptInterface PointProvider;
            int32 RequestID;
            int32 PreRegistrationHandle;
            TArray<FLivingWorldTagPreRegistration, TInlineAllocator<1>> TagPreRegistrations;
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
        float ActivationTime = -1.0f;
        int32 SpawnedCount = 0;
        float TotalPointProviderWeight = 0.0f;
        float DensityComputationRange = 0.0f;
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

    void ProcessEventRequests();

    void ProcessEQSRequests();

    void OnEQSRequestFinished(TSharedPtr<FEnvQueryResult> Result);

    void SpawnFromEQSRequest(const FLivingWorldPendingEQSSpawn& PendingEQSSpawn, const FVector* Location);

    void OnActorSpawned(AActor* SpawnedActor, int32 RequestID);

    void OnActorSpawned(FLivingWorldEventInstance& EventInstance, AActor* SpawnedActor, AActor* PointProvider, const FVector& PositionFromProvider, float DensityValue, float DensityRange);

    void ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight);

    bool IsEventActive(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData);

    int32 GetEventBudget(FFortAthenaLivingWorldEvent* Event);

    int32 GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event);

    int32 GetTotalEncounterInstanceActorCount(UFortAthenaLivingWorldEncounterInstance* EncounterInstance);

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

    TArray<FLivingWorldPendingActorSpawnEventRequest>& GetPendingActorSpawnEventRequests();

    float& GetNextEventGenerationTime();

    FGameplayTagContainer& GetPlaylistContextTags();

    UWorld** GetCachedWorld();

    AActor* GetEQSHelper();

    FFortAthenaLivingWorldDensityGrid* GetActorDensityGrid();

    TArray<AActor*>* GetSpawnedPawns();

    float GetActorDensity(const FVector& Location, float Radius);

    int32 GetActorDensityBudget(const FVector& Location, float Radius);

    float GetMaxActorDensity();

    void ClusterPoints(const TArray<FVector>& Points, TArray<FVector>& ClusteredPoints);

private:
    static TArray<FLivingWorldEventRuntimeDataEntry> EventRuntimeData;

    static TArray<FLivingWorldEventRequest> PendingEventRequests;

    static TArray<FLivingWorldPendingEQSSpawn> PendingEQSSpawns;

    static int32 NextEQSSpawnRequestID;
};
