#pragma once

#include "UObject/Object.h"

class IAbilitySystemInterface;

class UFortGlobals : public UObject
{
public:
    static void InitializePlayerGameplayAbilities(IAbilitySystemInterface* PlayerStateOrProxy);
};
