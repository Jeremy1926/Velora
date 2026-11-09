#pragma once

#include "Abilities/FortGameplayAbility.h"

struct FHitResult;

class UFortGameplayAbility_RangedWeapon : public UFortGameplayAbility
{
public:
    static UClass* StaticClass();

    void MakeTargetDataHandleFromHitResults(FGameplayAbilityTargetDataHandle& OutTargetData, const FHitResult& HitResult);
};
