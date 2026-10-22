#pragma once

#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AActor;
class UClass;
class UScriptStruct;

struct FFortAthenaLivingWorldPointProviderFilterRules
{
    FFortAthenaLivingWorldPointProviderFilterRules(bool bOnlyInSafeZone, bool bCheckForActorDensity, int32 ExpectedSpawnCount);

    static UScriptStruct* StaticStruct();

    static int32 GetFlagsOffset();

    int32 GetExpectedSpawnCount() const;

    bool GetbOnlyInSafeZone() const;

    bool GetbCheckForActorDensity() const;

    void SetbCheckForActorDensity(bool bCheckForActorDensity);

private:
    alignas(int32) uint8 Data[sizeof(int32) * 2];
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
