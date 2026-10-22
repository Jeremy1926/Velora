#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

class UEnvQueryInstanceBlueprintWrapper : public UObject
{
public:
    static UClass* StaticClass();

    void RunQuery(EEnvQueryRunMode::Type InRunMode, FEnvQueryRequest& QueryRequest);

    TArray<FVector> GetResultsAsLocations();

    void SetNamedParam(FName ParamName, float Value);
};
