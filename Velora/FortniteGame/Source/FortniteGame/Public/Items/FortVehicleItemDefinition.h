#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "UObject/SoftObjectPtr.h"

struct FScalableFloat;

class UFortVehicleItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    const FScalableFloat& GetVehicleMinSpawnPercent() const;

    const FScalableFloat& GetVehicleMaxSpawnPercent() const;

    const FScalableFloat& GetMinPercentWithGas() const;

    const FScalableFloat& GetMaxPercentWithGas() const;

    const FScalableFloat& GetMinPercentInoperable() const;

    const FScalableFloat& GetMaxPercentInoperable() const;

    const TSoftClassPtr<UClass>& GetVehicleActorClass() const;
};
