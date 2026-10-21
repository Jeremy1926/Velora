#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"
#include "UObject/WeakObjectPtr.h"
#include "Misc/Guid.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"

class UAbilitySystemComponent;
class UFortGameplayAbility;

struct FFortAbilitySetHandle
{
protected:
    TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
    TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;
    TArray<FGuid> ItemGuidsForAdditionalItems;
};

class UFortAbilitySet : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    TArray<TSubclassOf<UFortGameplayAbility>>& GetGameplayAbilities();
};
