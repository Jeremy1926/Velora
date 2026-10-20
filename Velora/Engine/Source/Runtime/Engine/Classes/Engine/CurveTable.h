#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class UCurveTable : public UObject
{
public:
    static UClass* StaticClass();
};

struct FCurveTableRowHandle
{
    const UCurveTable* CurveTable;
    FName RowName;
};
