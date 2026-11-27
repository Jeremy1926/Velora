#pragma once

#include "Building/BuildingActor.h"

struct FGameplayEffectContextHandle;

class ABuildingGameplayActor : public ABuildingActor
{
public:
    void K2_ExecuteGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);
};
