#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class FString;

struct FRealCurve
{
    float Eval(float InTime, float InDefaultValue = 0.0f) const;
};

class UCurveTable : public UObject
{
public:
    static UClass* StaticClass();

    FRealCurve* FindCurve(FName RowName, const FString& ContextString, bool bWarnIfNotFound = true) const;
};

struct FCurveTableRowHandle
{
    const UCurveTable* CurveTable;
    FName RowName;
};
