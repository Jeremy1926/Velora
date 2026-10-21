#pragma once

#include "GameFramework/Actor.h"

class AFortSafeZoneIndicator : public AActor
{
public:
    static UClass* StaticClass();

    void OffsetShrinkTimesFromServerTime(float Offset);

    void SetShrinkDuration(float ShrinkDuration);

    float GetShrinkDuration();

    float& GetSafeZoneStartShrinkTime();

    float& GetSafeZoneFinishShrinkTime();
};
