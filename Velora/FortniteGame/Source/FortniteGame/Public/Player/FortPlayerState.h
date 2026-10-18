#pragma once

#include "GameFramework/PlayerState.h"

class APawn;
class UFortAbilitySystemComponent;
class UFortRegenHealthSet;
class UFortControlResistanceSet;
class UFortDamageSet;
class UFortMovementSet;
class UFortConstructionSet;
class UFortPlayerAttrSet;
class UFortCharacterAttrSet;
class UFortWeaponAttrSet;
class UFortHomebaseSet;

struct FFortPlayerAttributeSets
{
    UFortRegenHealthSet* HealthSet;
    UFortControlResistanceSet* ControlResistanceSet;
    UFortDamageSet* DamageSet;
    UFortMovementSet* MovementSet;
    UFortConstructionSet* ConstructionSet;
    UFortPlayerAttrSet* PlayerAttrSet;
    UFortCharacterAttrSet* CharacterAttrSet;
    UFortWeaponAttrSet* WeaponAttrSet;
    UFortHomebaseSet* HomebaseSet;
};

class AFortPlayerState : public APlayerState
{
public:
    static UClass* StaticClass();

    void ApplyCharacterCustomization(APawn* Pawn);

    int32& GetWorldPlayerId();

    UFortAbilitySystemComponent*& GetAbilitySystemComponent();

    FFortPlayerAttributeSets& GetAttributeSets();
};
