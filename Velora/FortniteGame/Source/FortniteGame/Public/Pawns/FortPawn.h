#pragma once

#include "GameFramework/Character.h"

class AFortWeapon;
class UAbilitySystemComponent;
class UFortHealthSet;
struct FGameplayTag;

class AFortPawn : public ACharacter
{
public:
    static UClass* StaticClass();

    AFortWeapon* GetCurrentWeapon() const;

    UAbilitySystemComponent* GetAbilitySystemComponent() const;

    void StopPerformingNativeAction(const FGameplayTag& ActionTag);

    bool& GetIsDBNO();

    bool& GetPlayedDying();

    UFortHealthSet* GetHealthSet() const;
};
