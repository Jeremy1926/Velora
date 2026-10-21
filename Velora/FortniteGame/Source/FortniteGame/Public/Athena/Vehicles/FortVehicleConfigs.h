#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

struct FVehicleDamageablePartConfig
{
    FName ShapeName;
    FName BoneName;
    float MaxHealth;
};

class UFortVehicleConfigs : public UObject
{
public:
    static UClass* StaticClass();

    const TArray<FVehicleDamageablePartConfig>& GetDamageableParts() const;
};
