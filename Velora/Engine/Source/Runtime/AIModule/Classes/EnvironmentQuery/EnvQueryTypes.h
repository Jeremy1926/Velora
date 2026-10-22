#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Delegates/Delegate.h"
#include "Math/Vector.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class UClass;
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

struct FEnvQueryItem
{
    float Score;
    int32 DataOffset;
    uint8 bIsValid : 1;
};

struct FEnvQueryResult
{
    FVector GetItemAsLocation(int32 Index) const;

    TArray<FEnvQueryItem> Items;
    UClass* ItemType;
    TArray<uint8> RawData;
    uint8 Status;
    int32 OptionIndex;
    int32 QueryID;
};

typedef TDelegate<void(TSharedPtr<FEnvQueryResult>)> FQueryFinishedSignature;

struct FEnvQueryRequest
{
    FEnvQueryRequest(UEnvQuery* Query, UObject* RequestOwner)
        : QueryTemplate(Query)
        , Owner(RequestOwner)
        , World(nullptr)
    {
    }

    FEnvQueryRequest& SetFloatParam(FName ParamName, float Value);

    int32 Execute(EEnvQueryRunMode::Type RunMode, const FQueryFinishedSignature& FinishDelegate);

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
