#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class UScriptStruct;

struct FVehicleDamageablePartConfig
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    bool GetbStartsDisabled() const;

    FName GetShapeName() const;

    FName GetBoneName() const;

    float GetMaxHealth() const;
};

class UFortVehicleConfigs : public UObject
{
public:
    static UClass* StaticClass();

    const TRuntimeSizedArray<FVehicleDamageablePartConfig>& GetDamageableParts() const;
};
