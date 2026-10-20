#include "pch.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Scanner.h"

UEnvQueryInstanceBlueprintWrapper* UEnvQueryManager::RunEQSQuery(UObject* WorldContextObject, UEnvQuery* QueryTemplate, UObject* Querier, TEnumAsByte<EEnvQueryRunMode::Type> RunMode, TSubclassOf<UEnvQueryInstanceBlueprintWrapper> WrapperClass)
{
    static auto RunEQSQuery = Anchor::Function<UEnvQueryInstanceBlueprintWrapper* (*)(UObject*, UEnvQuery*, UObject*, TEnumAsByte<EEnvQueryRunMode::Type>, TSubclassOf<UEnvQueryInstanceBlueprintWrapper>)>(TEXT("UEnvQueryManager::RunEQSQuery"))
        .Native(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Get();

    return RunEQSQuery(WorldContextObject, QueryTemplate, Querier, RunMode, WrapperClass);
}
