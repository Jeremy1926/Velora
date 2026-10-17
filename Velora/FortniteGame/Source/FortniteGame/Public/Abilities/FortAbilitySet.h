#pragma once

#include "Engine/DataAsset.h"
#include "UObject/WeakObjectPtr.h"
#include "Containers/Array.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"

class IAbilitySystemInterface;
class UAbilitySystemComponent;

struct FFortAbilitySetHandle
{
protected:
    TWeakObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;
    TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
    TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;
};

class UFortAbilitySet : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    FFortAbilitySetHandle EquipAbilitySet(IAbilitySystemInterface* AbilitySystemInterface, UObject* OverrideSourceObject, IAbilitySystemInterface* SourceAbilitySystemInterface);
};
