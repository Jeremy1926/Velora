#pragma once

#include "UObject/NameTypes.h"
#include "Math/Vector.h"

class AActor;
class UScriptStruct;

struct alignas(8) FAINoiseEvent
{
    FAINoiseEvent(AActor* InInstigator, const FVector& InNoiseLocation, float InLoudness, float InMaxRange, FName InTag);

    static UScriptStruct* StaticStruct();

private:
    uint8 Data[0x38];
};
