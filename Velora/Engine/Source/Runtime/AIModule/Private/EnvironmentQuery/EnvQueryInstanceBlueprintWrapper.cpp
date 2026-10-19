#include "pch.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "Scanner.h"

TArray<FVector> UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations()
{
    static auto GetResultsAsLocations = Scanner::Function<TArray<FVector>* (*)(UEnvQueryInstanceBlueprintWrapper*, TArray<FVector>*)>(Scanner::Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:GetResultsAsLocations")).Get(TEXT("UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations")));

    TArray<FVector> Locations;
    GetResultsAsLocations(this, &Locations);

    return Locations;
}
