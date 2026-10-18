#include "pch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

float UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
    static auto EvaluateAttributeValueWithTagsAndBase = (float (*)(UAbilitySystemComponent*, FGameplayAttribute, const FGameplayTagContainer&, const FGameplayTagContainer&, float, bool&))Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase")).Find("movss [rsp+0x20], xmm*").Call().Get(TEXT("UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase"));

    return EvaluateAttributeValueWithTagsAndBase(AbilitySystem, Attribute, SourceTags, TargetTags, BaseValue, bSuccess);
}
