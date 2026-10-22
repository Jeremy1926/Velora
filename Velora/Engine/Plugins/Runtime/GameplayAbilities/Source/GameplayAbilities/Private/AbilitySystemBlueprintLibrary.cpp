#include "pch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UAbilitySystemComponent* UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AActor* Actor)
{
    static auto GetAbilitySystemComponent = Anchor::Function<UAbilitySystemComponent* (*)(AActor*, bool)>(TEXT("UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Get();

    return GetAbilitySystemComponent(Actor, false);
}

float UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
    static auto EvaluateAttributeValueWithTagsAndBase = Anchor::Function<float (*)(UAbilitySystemComponent*, FGameplayAttribute, const FGameplayTagContainer&, const FGameplayTagContainer&, float, bool&)>(TEXT("UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
        .Find("movss [rsp+0x20], xmm*")
        .Call()
        .Get();

    return EvaluateAttributeValueWithTagsAndBase(AbilitySystem, Attribute, SourceTags, TargetTags, BaseValue, bSuccess);
}
