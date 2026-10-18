#include "pch.h"
#include "AttributeSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

FGameplayAttribute::FGameplayAttribute(UProperty* NewProperty)
    : AttributeName(NewProperty ? NewProperty->GetName() : FString())
    , Attribute(NewProperty)
    , AttributeOwner(NewProperty ? (UStruct*)NewProperty->GetOuter() : nullptr)
{
}

float FGameplayAttributeData::GetCurrentValue() const
{
    return CurrentValue;
}
