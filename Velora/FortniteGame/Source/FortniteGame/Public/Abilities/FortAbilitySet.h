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
class UScriptStruct;

struct FFortAbilitySetHandle
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    bool IsValid() const
    {
        return TargetAbilitySystemComponent.IsValid();
    }

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

    static void UnequipAbilitySet(FFortAbilitySetHandle& Handle);

    TArray<TSubclassOf<UFortGameplayAbility>>& GetGameplayAbilities();
};
