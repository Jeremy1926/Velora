#pragma once

#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AActor;
class UClass;

struct FFortAthenaLivingWorldPointProviderFilterRules
{
    bool bOnlyInSafeZone;
    bool bCheckForActorDensity;
};

class IFortAthenaLivingWorldPointProviderInterface
{
public:
    static UClass* StaticClass();

    static bool HasActorDensityComputationRadius();

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const;

    void GetFiltersTags(FGameplayTagContainer& FilterTags) const;

    void OnSpawnedActor(AActor* SpawnedActor) const;

    void OnSpawnedActorFailed();

    void OnEventRequested();

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;
};
