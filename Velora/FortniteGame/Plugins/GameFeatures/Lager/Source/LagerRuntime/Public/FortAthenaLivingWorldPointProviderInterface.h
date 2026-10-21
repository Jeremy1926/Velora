#pragma once

#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class UClass;

struct FFortAthenaLivingWorldPointProviderFilterRules
{
    bool bOnlyInSafeZone;
};

class IFortAthenaLivingWorldPointProviderInterface
{
public:
    static UClass* StaticClass();

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, FVector& OutPosition, FRotator& OutRotation) const;

    void GetFiltersTags(FGameplayTagContainer& FilterTags) const;

    void OnSpawnedActorFailed();

    void OnEventRequested();

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;
};
