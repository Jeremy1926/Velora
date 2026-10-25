#pragma once

#include "GameFramework/Actor.h"

class AFortSafeZoneIndicatorFuture;

class AFortSafeZoneIndicator : public AActor
{
public:
    static UClass* StaticClass();

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

    AFortSafeZoneIndicatorFuture* GetFutureReplicator() const;
};
