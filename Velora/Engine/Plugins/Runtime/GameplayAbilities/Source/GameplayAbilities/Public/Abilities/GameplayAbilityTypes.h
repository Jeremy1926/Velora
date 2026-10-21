#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"
#include "GameplayAbilitySpec.h"
#include "GameplayPrediction.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

class AActor;
class UObject;
class UScriptStruct;

struct FGameplayEventData
{
    FGameplayEventData();

    FGameplayEventData(const FGameplayEventData&) = delete;

    ~FGameplayEventData();

    static UScriptStruct* StaticStruct();

    FGameplayTag EventTag;

    const AActor* Instigator;

    const AActor* Target;

    const UObject* OptionalObject;

    const UObject* OptionalObject2;

private:
    uint8 Data[0xD8];
};

struct FGameplayAbilitySpecHandleAndPredictionKey
{
    FGameplayAbilitySpecHandleAndPredictionKey()
        : PredictionKeyAtCreation(0)
    {
    }

    FGameplayAbilitySpecHandleAndPredictionKey(const FGameplayAbilitySpecHandle& HandleRef, const FPredictionKey& PredictionKeyAtCreationRef)
        : AbilityHandle(HandleRef), PredictionKeyAtCreation(PredictionKeyAtCreationRef.GetCurrent())
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

struct FGameplayAbilityReplicatedDataContainer
{
    FAbilityReplicatedDataCache* Find(const FGameplayAbilitySpecHandleAndPredictionKey& Key);

private:
    typedef TPair<FGameplayAbilitySpecHandleAndPredictionKey, TSharedRef<FAbilityReplicatedDataCache>> FKeyDataPair;

    TArray<FKeyDataPair> InUseData;
    TArray<TSharedRef<FAbilityReplicatedDataCache>> FreeData;
};
