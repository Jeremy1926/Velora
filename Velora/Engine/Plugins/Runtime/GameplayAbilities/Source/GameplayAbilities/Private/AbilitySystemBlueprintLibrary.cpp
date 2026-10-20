#include "pch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UAbilitySystemComponent* UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AActor* Actor)
{
    static auto GetAbilitySystemComponent = Scanner::Function<UAbilitySystemComponent* (*)(AActor*)>(Scanner::Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent")).Get(TEXT("UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent")));

    return GetAbilitySystemComponent(Actor);
}

float UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
    static auto EvaluateAttributeValueWithTagsAndBase = Scanner::Function<float (*)(UAbilitySystemComponent*, FGameplayAttribute, const FGameplayTagContainer&, const FGameplayTagContainer&, float, bool&)>(Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase")).Find("movss [rsp+0x20], xmm*").Call().Get(TEXT("UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase")));

    return EvaluateAttributeValueWithTagsAndBase(AbilitySystem, Attribute, SourceTags, TargetTags, BaseValue, bSuccess);
}
