#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class UEnvQueryInstanceBlueprintWrapper : public UObject
{
public:
    TArray<FVector> GetResultsAsLocations();

    void SetNamedParam(FName ParamName, float Value);
};
