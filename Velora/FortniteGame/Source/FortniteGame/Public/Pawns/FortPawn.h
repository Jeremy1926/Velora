#pragma once

#include "GameFramework/Character.h"
#include "Math/Vector.h"

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

    bool EquipBestWeapon();

    UAbilitySystemComponent* GetAbilitySystemComponent() const;

    void StopPerformingNativeAction(const FGameplayTag& ActionTag);

    bool GetIsDBNO() const;

    void SetIsDBNO(bool bInIsDBNO);

    void SetPlayedDying(bool bInPlayedDying);

    UFortHealthSet* GetHealthSet() const;

    void LaunchCharacterJump(const FVector& LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent);
};
