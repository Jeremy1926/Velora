#pragma once

#include "GameFramework/Actor.h"

class AFortSafeZoneIndicatorFuture;

struct FFortSafeZonePhaseInfo
{
    FVector3d Center;
    float Radius;
    float WaitTime;
    float ShrinkTime;
    int32 StormDamageLevel;
    int32 PlayerCap;
    float TimeBetweenStormCapDamage;
    float StormCapDamagePerTick;
    float StormCampingIncrementTimeAfterDelay;
    float StormCampingInitialDelayTime;
    int32 MegaStormGridCellThickness;
};

class AFortSafeZoneIndicator : public AActor
{
public:
    static UClass* StaticClass();

    void InitializeWithMapDefaultPhases();

    void StartPhase(int32 InPhaseIndex);

    bool ConstructMapDefaultPhase(int32 PhaseToConstruct, FFortSafeZonePhaseInfo& OutConstructedPhase) const;

    int32 GetPhaseDamageLevel() const;

    void SetSafeZoneRadiusAndCenter(float InRadius, const FVector& InLocation);

    void OffsetShrinkTimesFromServerTime(float Offset);

    void SetShrinkDuration(float ShrinkDuration);

    float GetShrinkDuration();

    void OnSafeZoneTimerPauseChanged(bool bIsPaused);

    float& GetSafeZoneStartShrinkTime();

    float& GetSafeZoneFinishShrinkTime();

    float& GetTimeRemainingWhenPhasePaused();

    float& GetNextNextRadius();

    FVector GetNextNextCenter();

    void SetNextNextCenter(const FVector& InNextNextCenter);

    void SetNextCenter(const FVector& InNextCenter);

    void SetRadius(float InRadius);

    float GetRadius();

    FVector GetNextCenter();

    int32 GetCurrentPhase() const;

    void SetCurrentPhase(int32 InCurrentPhase);

    TArray<FFortSafeZonePhaseInfo>& GetSafeZonePhases() const;

    AFortSafeZoneIndicatorFuture* GetFutureReplicator() const;
};
