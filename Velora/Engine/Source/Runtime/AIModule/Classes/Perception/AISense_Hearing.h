#pragma once

#include "UObject/NameTypes.h"
#include "Math/Vector.h"

class AActor;

struct alignas(8) FAINoiseEvent
{
    FAINoiseEvent(AActor* InInstigator, const FVector& InNoiseLocation, float InLoudness, float InMaxRange, FName InTag);

private:
    uint8 Data[0x38];
};
