#include "pch.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "Scanner.h"

TArray<FVector> UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations()
{
    static auto GetResultsAsLocations = Anchor::Function<TArray<FVector>* (*)(UEnvQueryInstanceBlueprintWrapper*, TArray<FVector>*)>(TEXT("UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations"))
        .Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:GetResultsAsLocations"))
        .Get();

    TArray<FVector> Locations;
    GetResultsAsLocations(this, &Locations);

    return Locations;
}

void UEnvQueryInstanceBlueprintWrapper::SetNamedParam(FName ParamName, float Value)
{
    static auto SetNamedParam = Anchor::Function<void (*)(UEnvQueryInstanceBlueprintWrapper*, FName, float)>(TEXT("UEnvQueryInstanceBlueprintWrapper::SetNamedParam"))
        .Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Get();

    SetNamedParam(this, ParamName, Value);
}
