#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "UObject/NameTypes.h"

class UScriptStruct;
class UEnvQuery;
class UObject;
class UWorld;

namespace EEnvQueryRunMode
{
    enum Type
    {
        SingleResult,
        RandomBest5Pct,
        RandomBest25Pct,
        AllMatching,
        EEnvQueryRunMode_MAX
    };
}

enum class EAIParamType : uint8
{
    Float,
    Int,
    Bool,
    MAX
};

struct FEnvQueryRequest
{
    FEnvQueryRequest(UEnvQuery* Query, UObject* RequestOwner)
        : QueryTemplate(Query)
        , Owner(RequestOwner)
        , World(nullptr)
    {
    }

    UEnvQuery* QueryTemplate;
    UObject* Owner;
    UWorld* World;
    TMap<FName, float> NamedParams;
};

struct FAIDynamicParam
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FName& GetParamName();

    EAIParamType& GetParamType();

    float& GetValue();
};

template<>
class TArray<FAIDynamicParam, FDefaultAllocator> : public TRuntimeSizedArray<FAIDynamicParam>
{
};
