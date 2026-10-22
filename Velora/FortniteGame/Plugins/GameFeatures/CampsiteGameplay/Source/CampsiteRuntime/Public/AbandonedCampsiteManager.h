#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "Math/Vector.h"

struct FFrame;
struct FScalableFloat;
class UEnvQuery;
class AAbandonedCampsitePlacedSpawner;
enum class EAthenaGamePhaseStep : uint8;

enum class EAbandonedCampsiteSpawnType : uint8
{
    None,
    PlacedInLevel,
    EnvironmentalQuery,
    FromPlayerProfile,
    Max
};

class AAbandonedCampsiteManager : public AActor
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void execOnGamePhaseStepChanged(FFrame& Stack, void* const Z_Param__Result);

    void OnGamePhaseStepChanged(EAthenaGamePhaseStep GamePhaseStep);

    void SpawnCampsites();

    void StartEnvironmentalQuerySpawns(int32 NumCampsites);

    void DeferredCampsiteSpawnLoop();

    bool IsFarEnoughFromSpawns(const FVector& Location, float MinimumDistanceSquared);

    static void TickDeferredCampsiteSpawns();

    FScalableFloat& GetMinNumAbandonedCampsites();

    FScalableFloat& GetMaxNumAbandonedCampsites();

    FScalableFloat& GetAbandonedCampsitesEnabled();

    const FScalableFloat* FindSpawnTypeWeight(EAbandonedCampsiteSpawnType SpawnType);

    UEnvQuery* GetSpawnEnvironmentalQuery();

    UClass* GetAbandonedCampsiteAsset();

    TArray<AAbandonedCampsitePlacedSpawner*>& GetPreplacedCampsiteSpawnPoints();

    TArray<AAbandonedCampsitePlacedSpawner*>& GetDeferredPreplacedCampsiteSpawns();
};
