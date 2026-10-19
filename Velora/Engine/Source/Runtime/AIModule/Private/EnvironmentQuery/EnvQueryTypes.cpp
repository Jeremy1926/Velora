#include "pch.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

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

FName& FAIDynamicParam::GetParamName()
{
    static auto ParamName = StaticStruct()->FindPropertyByName(FName(TEXT("ParamName")));

    return *ParamName->ContainerPtrToValuePtr<FName>(this);
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
