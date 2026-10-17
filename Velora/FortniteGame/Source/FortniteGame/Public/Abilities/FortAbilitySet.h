#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"

class UFortGameplayAbility;

class UFortAbilitySet : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    TArray<TSubclassOf<UFortGameplayAbility>>& GetGameplayAbilities();
};
