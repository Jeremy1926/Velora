#include "pch.h"
#include "GameplayEffect.h"
#include "Scanner.h"

bool FGameplayEffectAttributeCaptureSpec::AttemptCalculateAttributeMagnitudeWithBase(const FAggregatorEvaluateParameters& InEvalParams, float InBaseValue, float& OutMagnitude) const
{
    static auto AttemptCalculateAttributeMagnitudeWithBase = Anchor::Function<bool (*)(const FGameplayEffectAttributeCaptureSpec*, const FAggregatorEvaluateParameters&, float, float&)>(TEXT("FGameplayEffectAttributeCaptureSpec::AttemptCalculateAttributeMagnitudeWithBase"))
        .Pattern("48 8B 59 40 49 8B F9 0F 29 74 24 20 0F 28 F2 48 85 DB")
        .Start()
        .Get();

    return AttemptCalculateAttributeMagnitudeWithBase(this, InEvalParams, InBaseValue, OutMagnitude);
}

void FGameplayEffectSpec::SetSetByCallerMagnitude(FName DataName, float Magnitude)
{
    static auto SetSetByCallerMagnitude = Anchor::Function<void (*)(FGameplayEffectSpec*, FName, float)>(TEXT("FGameplayEffectSpec::SetSetByCallerMagnitude"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:AssignSetByCallerMagnitude"))
        .Call()
        .Get();

    SetSetByCallerMagnitude(this, DataName, Magnitude);
}

void FGameplayEffectSpec::SetSetByCallerMagnitude(FGameplayTag DataTag, float Magnitude)
{
    static auto SetSetByCallerMagnitude = Anchor::Function<void (*)(FGameplayEffectSpec*, FGameplayTag, float)>(TEXT("FGameplayEffectSpec::SetSetByCallerMagnitude(FGameplayTag)"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:AssignTagSetByCallerMagnitude"))
        .Call()
        .Get();

    SetSetByCallerMagnitude(this, DataTag, Magnitude);
}
