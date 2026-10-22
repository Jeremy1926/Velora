#pragma once

#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class AFortGameStateAthena;
class UEnvQuery;
struct FEnvQueryResult;
struct FFortAthenaLivingWorldPointProviderFilterRules;
struct FFrame;
struct FGameplayTagContainer;

class AFortAthenaLivingWorldVolume : public AActor
{
public:
    static UClass* StaticClass();

    void OnCurrentPlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags);

    void execOnCurrentPlaylistLoaded(FFrame& Stack, void* const Z_Param__Result);

    void OnEnvQueryFinished(TSharedPtr<FEnvQueryResult> Result);

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, FVector& OutPosition, FRotator& OutRotation) const;

    AFortGameStateAthena* GetCachedGameState() const;

    UEnvQuery* GetEnvironmentQuery() const;

    const FScalableFloat& GetWaterLevelIndexMin() const;

    const FScalableFloat& GetWaterLevelIndexMax() const;

    TArray<FVector>& GetPoints() const;
};
