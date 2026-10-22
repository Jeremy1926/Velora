#pragma once

#include "UObject/Object.h"
#include "Containers/EnumAsByte.h"
#include "Templates/SubclassOf.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;

class UEnvQueryManager : public UObject
{
public:
    static UEnvQueryManager* GetCurrent(UObject* WorldContextObject);

    static UEnvQueryInstanceBlueprintWrapper* RunEQSQuery(UObject* WorldContextObject, UEnvQuery* QueryTemplate, UObject* Querier, TEnumAsByte<EEnvQueryRunMode::Type> RunMode, TSubclassOf<UEnvQueryInstanceBlueprintWrapper> WrapperClass);
};
