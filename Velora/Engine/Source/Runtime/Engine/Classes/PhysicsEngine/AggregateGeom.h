#pragma once

#include "UObject/NameTypes.h"

class UScriptStruct;

struct FKAggregateGeom
{
    static UScriptStruct* StaticStruct();

    int32 GetElementIndexByName(const FName InName) const;
};
