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

void UEnvQueryInstanceBlueprintWrapper::SetNamedParam(FName ParamName, float Value)
{
    static auto SetNamedParam = Scanner::Function<void (*)(UEnvQueryInstanceBlueprintWrapper*, FName, float)>(Scanner::Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam")).Get(TEXT("UEnvQueryInstanceBlueprintWrapper::SetNamedParam")));

    SetNamedParam(this, ParamName, Value);
}
