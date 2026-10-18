#pragma once

#include "AttributeSet.h"

struct FFortGameplayAttributeData : public FGameplayAttributeData
{
};

class UFortHealthSet : public UAttributeSet
{
public:
    const FFortGameplayAttributeData& GetHealth() const;
};
