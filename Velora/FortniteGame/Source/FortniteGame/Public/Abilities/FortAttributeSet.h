#pragma once

#include "AttributeSet.h"

class UFortAttributeSet : public UAttributeSet
{
public:
    void SetNumericAttributeBase(const FGameplayAttribute& Attribute, float NewValue);
};
