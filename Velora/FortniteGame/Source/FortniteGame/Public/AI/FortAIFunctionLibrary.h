#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"

class AActor;

class UFortAIFunctionLibrary : public UBlueprintFunctionLibrary
{
public:
    static void MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation);
};
