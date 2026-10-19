#pragma once

#include "UObject/Object.h"

class IAbilitySystemInterface;

class UFortRegisteredPlayerInfo : public UObject
{
public:
    IAbilitySystemInterface* UpdateAbilitySystemActor(bool bForceUpdate);

    IAbilitySystemInterface* PlayerPawnReadyUpdateAbilitySystemActor(bool bForceUpdate);
};
