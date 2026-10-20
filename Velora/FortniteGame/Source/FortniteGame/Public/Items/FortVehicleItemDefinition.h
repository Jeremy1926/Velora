#pragma once

#include "Items/FortWorldItemDefinition.h"

struct FScalableFloat;

class UFortVehicleItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    const FScalableFloat& GetVehicleMinSpawnPercent() const;

    const FScalableFloat& GetVehicleMaxSpawnPercent() const;
};
