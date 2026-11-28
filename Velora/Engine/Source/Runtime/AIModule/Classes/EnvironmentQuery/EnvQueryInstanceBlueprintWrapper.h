#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Delegates/Delegate.h"
#include "Templates/SharedPointer.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

class UEnvQueryInstanceBlueprintWrapper : public UObject
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, UEnvQueryInstanceBlueprintWrapper*>& GetNotifyQueryFinished();

    void RunQuery(EEnvQueryRunMode::Type InRunMode, FEnvQueryRequest& QueryRequest);

    void OnQueryFinished(TSharedPtr<FEnvQueryResult> Result);

    TArray<FVector> GetResultsAsLocations();

    void SetNamedParam(FName ParamName, float Value);
};
