#pragma once

#include "Containers/Map.h"
#include "GameplayTasksComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayPrediction.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

class UAbilitySystemComponent : public UGameplayTasksComponent
{
public:
    FGameplayAbilitySpecHandle GiveAbility(const FGameplayAbilitySpec& AbilitySpec);

    FGameplayAbilitySpec* FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle);

    void MarkAbilitySpecDirty(FGameplayAbilitySpec& Spec);

    bool InternalTryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate, FPredictionKey InPredictionKey = FPredictionKey(), UGameplayAbility** OutInstancedAbility = nullptr, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr, const FGameplayEventData* TriggerEventData = nullptr);

    static FGameplayTagContainer& InternalTryActivateAbilityFailureTags;

    void ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey);

    void InternalServerTryActiveAbility(FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData);

    void ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey);

    FGameplayAbilitySpecContainer& GetActivatableAbilities();

    TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>& GetAbilityTargetDataMap();
};
