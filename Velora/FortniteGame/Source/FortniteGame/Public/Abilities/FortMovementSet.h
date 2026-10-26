#pragma once

#include "Abilities/FortHealthSet.h"

class UFortMovementSet : public UAttributeSet
{
public:
    static UClass* StaticClass();

    FFortGameplayAttributeData* GetFlySpeed();

    FFortGameplayAttributeData& GetSpeedMultiplier();
};
