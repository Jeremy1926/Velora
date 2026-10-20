#pragma once

#include "Containers/UnrealString.h"
#include "UObject/Object.h"
#include "UObject/FieldPath.h"

class FProperty;
class UProperty;
class UStruct;

struct FGameplayAttributeData
{
    virtual ~FGameplayAttributeData()
    {
    }

    float GetCurrentValue() const;

protected:
    float BaseValue;
    float CurrentValue;
};

struct FGameplayAttribute
{
    FGameplayAttribute(UProperty* NewProperty);

    FString AttributeName;

private:
    TFieldPath<FProperty> Attribute;
    UStruct* AttributeOwner;
};

class UAttributeSet : public UObject
{
};
