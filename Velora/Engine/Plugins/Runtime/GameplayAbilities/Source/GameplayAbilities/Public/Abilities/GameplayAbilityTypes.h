#pragma once

#include "HAL/Platform.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"

struct FGameplayEventData;

struct FGameplayAbilitySpecHandleAndPredictionKey
{
    FGameplayAbilitySpecHandleAndPredictionKey()
        : PredictionKeyAtCreation(0)
    {
    }

    FGameplayAbilitySpecHandleAndPredictionKey(const FGameplayAbilitySpecHandle& HandleRef, const FPredictionKey& PredictionKeyAtCreationRef)
        : AbilityHandle(HandleRef), PredictionKeyAtCreation(PredictionKeyAtCreationRef.Current)
    {
    }

    bool operator==(const FGameplayAbilitySpecHandleAndPredictionKey& Other) const
    {
        return AbilityHandle == Other.AbilityHandle && PredictionKeyAtCreation == Other.PredictionKeyAtCreation;
    }

    bool operator!=(const FGameplayAbilitySpecHandleAndPredictionKey& Other) const
    {
        return AbilityHandle != Other.AbilityHandle || PredictionKeyAtCreation != Other.PredictionKeyAtCreation;
    }

    friend uint32 GetTypeHash(const FGameplayAbilitySpecHandleAndPredictionKey& Handle)
    {
        return GetTypeHash(Handle.AbilityHandle) ^ Handle.PredictionKeyAtCreation;
    }

private:
    FGameplayAbilitySpecHandle AbilityHandle;
    int32 PredictionKeyAtCreation;
};
