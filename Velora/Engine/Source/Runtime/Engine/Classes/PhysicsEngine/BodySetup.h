#pragma once

#include "UObject/Object.h"

struct FKAggregateGeom;

class UBodySetup : public UObject
{
public:
    static UClass* StaticClass();

    FKAggregateGeom& GetAggGeom();
};
