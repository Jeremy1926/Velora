#pragma once

#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AActor;
class UClass;
class UFortAthenaLivingWorldEventData;
class UScriptStruct;
struct FFortAthenaLivingWorldEvent;

struct FFortAthenaLivingWorldPointProviderFilterRules
{
    FFortAthenaLivingWorldPointProviderFilterRules(bool bOnlyInSafeZone, bool bCheckForActorDensity, int32 ExpectedSpawnCount);

    FFortAthenaLivingWorldPointProviderFilterRules(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData, int32 ExpectedSpawnCount, float DensityComputationRange, float MaxActorDensity);

    static UScriptStruct* StaticStruct();

    static int32 GetFlagsOffset();

    static bool HasEventRules();

    static bool HasDensityRules();

    static int32 GetPlayerDistanceOffset();

    int32 GetExpectedSpawnCount() const;

    bool GetbOnlyInSafeZone() const;

    bool GetbCheckForActorDensity() const;

    void SetbCheckForActorDensity(bool bCheckForActorDensity);

private:
    alignas(void*) uint8 Data[0x30];
};

struct FFortAthenaLivingWorldPointProviderSpawnLimiter
{
    static UScriptStruct* StaticStruct();

    static FFortAthenaLivingWorldPointProviderSpawnLimiter* Get(const AActor* PointProvider);

    bool CanSpawn(int32 ExpectedSpawnCount);

    int32 CurrentNumberOfSpawn;
};

class IFortAthenaLivingWorldPointProviderInterface
{
public:
    static UClass* StaticClass();

    static bool HasActorDensityComputationRadius();

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const;

    void GetFiltersTags(FGameplayTagContainer& FilterTags) const;

    void OnSpawnedActor(AActor* SpawnedActor, const FVector& PositionFromProvider) const;

    void OnSpawnedActorFailed();

    void OnEventRequested();

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;
};
