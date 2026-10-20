#pragma once

#include "GameFramework/Character.h"

class AFortWeapon;
class UAbilitySystemComponent;
class UFortHealthSet;
class UFortWeaponItemDefinition;
struct FGameplayTag;
struct FGuid;

class AFortPawn : public ACharacter
{
public:
    static UClass* StaticClass();

    AFortWeapon* GetCurrentWeapon() const;

    AFortWeapon* EquipWeaponDefinition(const UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid);

    UAbilitySystemComponent* GetAbilitySystemComponent() const;

    void StopPerformingNativeAction(const FGameplayTag& ActionTag);

    bool GetIsDBNO() const;

    void SetIsDBNO(bool bInIsDBNO);

    void SetPlayedDying(bool bInPlayedDying);

    UFortHealthSet* GetHealthSet() const;
};
