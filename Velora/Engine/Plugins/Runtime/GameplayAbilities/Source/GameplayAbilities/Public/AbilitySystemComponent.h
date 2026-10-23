#pragma once

#include "Containers/Map.h"
#include "Templates/SubclassOf.h"
#include "GameplayTasksComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayPrediction.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayEffectTypes.h"

class UGameplayEffect;
struct FGameplayEffectAttributeCaptureSpec;
struct FActiveGameplayCueContainer;
struct FGameplayEffectSpec;

template<>
class TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>
{
public:
    FAbilityReplicatedDataCache* Find(const FGameplayAbilitySpecHandleAndPredictionKey& Key);

private:
    TArray<uint8> Elements;
    TBitArray<> AllocationFlags;
    int32 FirstFreeIndex;
    int32 NumFreeIndices;
    TInlineAllocator<1>::ForElementType<FSetElementId> Hash;
    int32 HashSize;
};

class UAbilitySystemComponent : public UGameplayTasksComponent
{
public:
    static UClass* StaticClass();

    FGameplayEffectContextHandle MakeEffectContext() const;

    bool HasAttributeSetForAttribute(FGameplayAttribute Attribute) const;

    void CaptureAttributeForGameplayEffect(FGameplayEffectAttributeCaptureSpec& OutCaptureSpec);

    FGameplayEffectSpecHandle MakeOutgoingSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle Context) const;

    FActiveGameplayEffectHandle ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& GameplayEffect, FPredictionKey PredictionKey = FPredictionKey());

    FActiveGameplayEffectHandle BP_ApplyGameplayEffectToTarget(TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level, FGameplayEffectContextHandle Context);

    FActiveGameplayEffectHandle BP_ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle EffectContext);

    FORCEINLINE void RemoveLooseGameplayTag(const FGameplayTag& GameplayTag, int32 Count = 1)
    {
        UpdateTagMap(GameplayTag, -Count);
    }

    FORCEINLINE void UpdateTagMap(const FGameplayTag& BaseTag, int32 CountDelta)
    {
        if (GetGameplayTagCountContainer().UpdateTagCount(BaseTag, CountDelta))
        {
            OnTagUpdated(BaseTag, CountDelta > 0);
        }
    }

    void OnTagUpdated(const FGameplayTag& Tag, bool TagExists);

    void RemoveGameplayCue(const FGameplayTag GameplayCueTag);

    void RemoveGameplayCue_Internal(const FGameplayTag GameplayCueTag, FActiveGameplayCueContainer& GameplayCueContainer);

    void CancelAbilities(const FGameplayTagContainer* WithTags = nullptr, const FGameplayTagContainer* WithoutTags = nullptr, UGameplayAbility* Ignore = nullptr);

    int32 HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);

    FActiveGameplayCueContainer& GetActiveGameplayCues();

    FGameplayTagCountContainer& GetGameplayTagCountContainer();

    FGameplayAbilitySpecHandle GiveAbility(const FGameplayAbilitySpec& AbilitySpec);

    FGameplayAbilitySpecHandle GiveAbilityAndActivateOnce(const FGameplayAbilitySpec& AbilitySpec, const FGameplayEventData* GameplayEventData = nullptr);

    void ClearAbility(const FGameplayAbilitySpecHandle& Handle);

    FGameplayAbilitySpec* FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle);

    FGameplayAbilitySpec* FindAbilitySpecFromClass(TSubclassOf<UGameplayAbility> InAbilityClass);

    void MarkAbilitySpecDirty(FGameplayAbilitySpec& Spec, bool WasAddOrRemove = false);

    bool InternalTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey = FPredictionKey(), UGameplayAbility** OutInstancedAbility = nullptr, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr, const FGameplayEventData* TriggerEventData = nullptr);

    FGameplayTagContainer& GetInternalTryActivateAbilityFailureTags();

    void ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey);

    void InternalServerTryActiveAbility(FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData);

    void ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey);

    FGameplayAbilitySpecContainer& GetActivatableAbilities();

    TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>& GetAbilityTargetDataMap();
};
