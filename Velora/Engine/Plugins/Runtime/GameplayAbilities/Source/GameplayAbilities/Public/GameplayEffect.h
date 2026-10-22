#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectAggregator.h"

class UGameplayEffect : public UObject
{
};

struct FGameplayEffectAttributeCaptureSpec
{
    FGameplayEffectAttributeCaptureSpec(const FGameplayEffectAttributeCaptureDefinition& InDefinition)
        : BackingDefinition(InDefinition)
    {
    }

    bool AttemptCalculateAttributeMagnitudeWithBase(const FAggregatorEvaluateParameters& InEvalParams, float InBaseValue, float& OutMagnitude) const;

private:
    FGameplayEffectAttributeCaptureDefinition BackingDefinition;
    FAggregatorRef AttributeAggregator;
};

struct FGameplayEffectSpec
{
    void SetSetByCallerMagnitude(FName DataName, float Magnitude);

    void SetSetByCallerMagnitude(FGameplayTag DataTag, float Magnitude);
};
