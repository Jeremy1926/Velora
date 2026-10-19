#pragma once

#include "UObject/Object.h"
#include "Delegates/Delegate.h"

class AActor;
class UGameplayAbility;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayAbilityEnded, UGameplayAbility*);

class UGameplayAbility : public UObject
{
public:
    AActor* GetAvatarActorFromActorInfo() const;
};
