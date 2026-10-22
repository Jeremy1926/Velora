#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class UFortAthenaAIRuntimeParameters_Inventory : public UObject
{
public:
    static UClass* StaticClass();

    bool GetbShouldDropCurrencyOnDeath() const;

    FName GetCurrencyPayoutRowName() const;
};
