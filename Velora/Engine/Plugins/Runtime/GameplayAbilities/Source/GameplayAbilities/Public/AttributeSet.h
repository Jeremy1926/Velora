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

    virtual void SetCurrentValue(float NewValue);

    virtual void SetBaseValue(float NewValue);

protected:
    float BaseValue;
    float CurrentValue;
};

struct FGameplayAttribute
{
    FGameplayAttribute(UProperty* NewProperty);

    bool operator==(const FGameplayAttribute& Other) const
    {
        return AttributeOwner == Other.AttributeOwner && AttributeName == Other.AttributeName;
    }

    FString AttributeName;

private:
    TFieldPath<FProperty> Attribute;
    UStruct* AttributeOwner;
};

class UAttributeSet : public UObject
{
};
