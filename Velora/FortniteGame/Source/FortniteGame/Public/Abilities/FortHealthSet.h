#pragma once

#include "Abilities/FortAttributeSet.h"

struct FFortGameplayAttributeData : public FGameplayAttributeData
{
};

class UFortHealthSet : public UFortAttributeSet
{
public:
    static UClass* StaticClass();

    static FGameplayAttribute GetHealthAttribute();

    static FGameplayAttribute GetCurrentShieldAttribute();

    const FFortGameplayAttributeData& GetHealth() const;

    const FFortGameplayAttributeData& GetMaxHealth() const;
};
