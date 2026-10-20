#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

class AActor;
class UAbilitySystemComponent;

class UAbilitySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
public:
    static UAbilitySystemComponent* GetAbilitySystemComponent(AActor* Actor);

    static float EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess);
};
