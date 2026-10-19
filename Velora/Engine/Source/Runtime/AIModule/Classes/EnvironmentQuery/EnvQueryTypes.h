#pragma once

#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class UScriptStruct;

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
