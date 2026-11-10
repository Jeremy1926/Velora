#pragma once

#include "AbilitySystemComponent.h"

class UFortAbilitySystemComponent : public UAbilitySystemComponent
{
public:
    static UClass* StaticClass();

    void OnTagUpdated(const FGameplayTag& Tag, bool TagExists);
};
