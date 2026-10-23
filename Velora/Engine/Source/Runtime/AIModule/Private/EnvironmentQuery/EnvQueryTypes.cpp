#include "pch.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

FVector FEnvQueryResult::GetItemAsLocation(int32 Index) const
{
    static auto GetItemAsLocation = Anchor::Function<FVector* (*)(const FEnvQueryResult*, FVector*, int32)>(TEXT("FEnvQueryResult::GetItemAsLocation"))
        .Native(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:GetResultsAsLocations"))
        .Find("cmp dword ptr [*+0x*], 0x3; jz 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    FVector Location;

    return *GetItemAsLocation(this, &Location, Index);
}

FEnvQueryRequest& FEnvQueryRequest::SetFloatParam(FName ParamName, float Value)
{
    static auto Add = Anchor::Function<float* (*)(TMap<FName, float>*, FName*, float*)>(TEXT("TMap<FName, float>::Add"))
        .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Get();

    Add(&NamedParams, &ParamName, &Value);

    return *this;
}

UScriptStruct* FAIDynamicParam::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.AIDynamicParam"));

    return Struct;
}

int32 FAIDynamicParam::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FName FAIDynamicParam::GetParamName()
{
    static auto ParamName = StaticStruct()->FindPropertyByName(FName(TEXT("ParamName")));

    return ParamName->GetPropertyValue_InContainer<FName>(this);
}

EAIParamType& FAIDynamicParam::GetParamType()
{
    static auto ParamType = StaticStruct()->FindPropertyByName(FName(TEXT("ParamType")));

    return *ParamType->ContainerPtrToValuePtr<EAIParamType>(this);
}

float& FAIDynamicParam::GetValue()
{
    static auto Value = StaticStruct()->FindPropertyByName(FName(TEXT("Value")));

    return *Value->ContainerPtrToValuePtr<float>(this);
}
