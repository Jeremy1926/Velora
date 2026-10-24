#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortGameStateAthena;
struct FFrame;
struct FFortAthenaLivingWorldPointProviderFilterRules;

class AFortAthenaPatrolPathPointProvider : public AActor
{
public:
    static UClass* StaticClass();

    bool IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const;

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, TGameValue<FVector>& GameOutPosition, TGameValue<FRotator>& GameOutRotation) const;

    bool GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, TGameValue<FVector>& GameOutPosition, TGameValue<FRotator>& GameOutRotation) const;

    void OnSpawnedActor(AActor* SpawnedActor);

    void OnCurrentPlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags);

    void EnablePointProvider();

    void DisablePointProvider();

    void InitPointProvider(AActor* PatrolPath, bool bUsePatrolPathTags);

    void execDisablePointProvider(FFrame& Stack, void* const Z_Param__Result);

    AFortGameStateAthena* GetCachedGameState() const;

    AActor* GetRuntimePatrolPath() const;

    AActor* GetAssociatedPatrolPath() const;

    bool GetbStartEnabled() const;

    bool GetbSpawnOnPatrolPath() const;

    bool GetbDisableOnSuccessfulSpawn() const;

    float GetRadius() const;
};
