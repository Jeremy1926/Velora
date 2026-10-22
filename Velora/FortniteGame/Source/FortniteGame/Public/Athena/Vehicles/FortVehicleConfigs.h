#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class UScriptStruct;

struct FVehicleDamageablePartConfig
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FName ShapeName;
    FName BoneName;
    float MaxHealth;
};

class UFortVehicleConfigs : public UObject
{
public:
    static UClass* StaticClass();

    const TRuntimeSizedArray<FVehicleDamageablePartConfig>& GetDamageableParts() const;
};
