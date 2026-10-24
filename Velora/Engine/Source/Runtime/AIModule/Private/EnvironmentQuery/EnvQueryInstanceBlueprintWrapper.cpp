#include "pch.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UEnvQueryInstanceBlueprintWrapper::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper"));

    return Class;
}

void UEnvQueryInstanceBlueprintWrapper::RunQuery(EEnvQueryRunMode::Type InRunMode, FEnvQueryRequest& QueryRequest)
{
    static auto RunQuery = Anchor::Function<void (*)(UEnvQueryInstanceBlueprintWrapper*, uint8, FEnvQueryRequest*)>(TEXT("UEnvQueryInstanceBlueprintWrapper::RunQuery"))
        .Exec(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("lea r8, [*]; *; mov edx, *")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; lea rcx, [*]; call 0x*")
        .Target()
        .Get();

    RunQuery(this, (uint8)InRunMode, &QueryRequest);
}

TArray<FVector> UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations()
{
    static auto GetResultsAsLocations = Anchor::Function<TArray<FVector>* (*)(UEnvQueryInstanceBlueprintWrapper*, TArray<FVector>*)>(TEXT("UEnvQueryInstanceBlueprintWrapper::GetResultsAsLocations"))
        .Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:GetResultsAsLocations"))
        .Get();

    TArray<FVector> Locations;

    if (!UE::LWC::IsEnabled())
    {
        GetResultsAsLocations(this, &Locations);

        return Locations;
    }

    TArray<FVector3d> GameLocations;
    ((TArray<FVector3d>* (*)(UEnvQueryInstanceBlueprintWrapper*, TArray<FVector3d>*))GetResultsAsLocations)(this, &GameLocations);

    for (auto& GameLocation : GameLocations)
        Locations.Add((FVector)GameLocation);

    return Locations;
}

void UEnvQueryInstanceBlueprintWrapper::SetNamedParam(FName ParamName, float Value)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto QueryInstance = Anchor::Offset(TEXT("UEnvQueryInstanceBlueprintWrapper::QueryInstance"))
            .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov rcx, [*+0x*]")
            .Disp();

        static auto NamedParams = Anchor::Offset(TEXT("FEnvQueryInstance::NamedParams"))
            .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("add rcx, 0x*")
            .Imm();

        static auto Add = Anchor::Function<float* (*)(TMap<FName, float>*, FName*, float*)>(TEXT("TMap<FName, float>::Add"))
            .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Get();

        auto Instance = QueryInstance ? *(uint8**)((uint8*)this + QueryInstance) : nullptr;

        if (Instance && NamedParams)
            Add((TMap<FName, float>*)(Instance + NamedParams), &ParamName, &Value);

        return;
    }

    static auto SetNamedParam = Anchor::Function<void (*)(UEnvQueryInstanceBlueprintWrapper*, FName, float)>(TEXT("UEnvQueryInstanceBlueprintWrapper::SetNamedParam"))
        .Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Get();

    SetNamedParam(this, ParamName, Value);
}
