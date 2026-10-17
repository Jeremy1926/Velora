#include "pch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

float UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
    static auto EvaluateAttributeValueWithTagsAndBase = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"));
    static auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__AbilitySystemBlueprintLibrary"));

    struct
    {
        UAbilitySystemComponent* AbilitySystem;
        FGameplayAttribute Attribute;
        FGameplayTagContainer SourceTags;
        FGameplayTagContainer TargetTags;
        float BaseValue;
        bool bSuccess;
        float ReturnValue;
    } Parms{ AbilitySystem, Attribute, SourceTags, TargetTags, BaseValue };

    DefaultObject->ProcessEvent(EvaluateAttributeValueWithTagsAndBase, &Parms);

    bSuccess = Parms.bSuccess;

    return Parms.ReturnValue;
}
