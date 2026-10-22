#pragma once

#include "UObject/NameTypes.h"
#include "Math/Vector.h"

class AActor;

struct FAINoiseEvent
{
    FAINoiseEvent(AActor* InInstigator, const FVector& InNoiseLocation, float InLoudness, float InMaxRange, FName InTag);

    float Age;
    FVector NoiseLocation;
    float Loudness;
    float MaxRange;
    AActor* Instigator;
    FName Tag;
    uint8 EventID;
};
