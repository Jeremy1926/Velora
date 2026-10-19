#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Math/Vector.h"

class UEnvQueryInstanceBlueprintWrapper : public UObject
{
public:
    TArray<FVector> GetResultsAsLocations();
};
