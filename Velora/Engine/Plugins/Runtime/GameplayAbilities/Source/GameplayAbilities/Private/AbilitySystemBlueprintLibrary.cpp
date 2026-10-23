#include "pch.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UAbilitySystemComponent* UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AActor* Actor)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; mov rdx, rax; mov rcx, *; call 0x*; test rax, rax")
        .Get();

    if (bInlined)
    {
        static auto GetAbilitySystemComponentFromInterface = Anchor::VirtualFunction<UAbilitySystemComponent* (*)(const void*)>(TEXT("IAbilitySystemInterface::GetAbilitySystemComponent"))
            .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [*+0x*]")
            .Get();

        if (!Actor)
            return nullptr;

        static auto AbilitySystemInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemInterface"));

        if (auto AbilitySystemInterfaceAddress = Actor->GetInterfaceAddress(AbilitySystemInterface))
            return GetAbilitySystemComponentFromInterface(AbilitySystemInterfaceAddress);

        return Actor->FindComponentByClass<UAbilitySystemComponent>();
    }

    static auto GetAbilitySystemComponent = Anchor::Function<UAbilitySystemComponent* (*)(AActor*, bool)>(TEXT("UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Get();

    return GetAbilitySystemComponent(Actor, false);
}

float UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase(UAbilitySystemComponent* AbilitySystem, FGameplayAttribute Attribute, const FGameplayTagContainer& SourceTags, const FGameplayTagContainer& TargetTags, float BaseValue, bool& bSuccess)
{
    static auto EvaluateAttributeValueWithTagsAndBase = Anchor::Optional<float (*)(UAbilitySystemComponent*, FGameplayAttribute, const FGameplayTagContainer&, const FGameplayTagContainer&, float, bool&)>(TEXT("UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
        .Find("movss [rsp+0x20], xmm*")
        .Call()
        .Get();

    if (EvaluateAttributeValueWithTagsAndBase)
        return EvaluateAttributeValueWithTagsAndBase(AbilitySystem, Attribute, SourceTags, TargetTags, BaseValue, bSuccess);

    float RetVal = 0.0f;

    if (!AbilitySystem || !AbilitySystem->HasAttributeSetForAttribute(Attribute))
    {
        bSuccess = false;
        return RetVal;
    }

    FGameplayEffectAttributeCaptureDefinition Capture(Attribute, EGameplayEffectAttributeCaptureSource::Source, true);

    FGameplayEffectAttributeCaptureSpec CaptureSpec(Capture);
    AbilitySystem->CaptureAttributeForGameplayEffect(CaptureSpec);

    FAggregatorEvaluateParameters EvalParams;

    EvalParams.SourceTags = &SourceTags;
    EvalParams.TargetTags = &TargetTags;

    bSuccess = CaptureSpec.AttemptCalculateAttributeMagnitudeWithBase(EvalParams, BaseValue, RetVal);

    return RetVal;
}
