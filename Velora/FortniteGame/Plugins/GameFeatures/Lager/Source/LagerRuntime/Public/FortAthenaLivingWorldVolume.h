#pragma once

#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class AFortGameStateAthena;
class UEnvQuery;
class UFortAthenaLivingWorldManager;
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

    void EnablePointProvider();

    void DisablePointProvider();

    void OnEnvQueryFinished(TSharedPtr<FEnvQueryResult> Result);

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, TGameValue<FVector>& GameOutPosition, TGameValue<FRotator>& GameOutRotation) const;

    AFortGameStateAthena*& GetCachedGameState() const;

    UFortAthenaLivingWorldManager** GetCachedLivingWorldManager();

    bool* GetbIsEnabled();

    void SetClusteredEnvironmentQueryResults(const TArray<FVector>& InClusteredEnvironmentQueryResults);

    float* GetPointProviderMaxRadiusSqr();

    UEnvQuery* GetEnvironmentQuery() const;

    const FScalableFloat& GetWaterLevelIndexMin() const;

    const FScalableFloat& GetWaterLevelIndexMax() const;

    int32 GetPointsOffset() const;

    TArray<FVector> GetPoints() const;

    void SetPoints(const TArray<FVector>& InPoints);
};
