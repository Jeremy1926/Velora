#pragma once

#include "GameFramework/Character.h"
#include "Math/Vector.h"
#include "UObject/ScriptDelegates.h"

class AFortWeapon;
class UAbilitySystemComponent;
class UFortHealthSet;
class UFortItemDefinition;
class UFortWeaponItemDefinition;
struct FFrame;
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

    bool GetWasDBNOOnDeath() const;

    void SetPlayedDying(bool bInPlayedDying);

    void SetMovingEmote(bool bInMovingEmote);

    void SetMovingEmoteForwardOnly(bool bInMovingEmoteForwardOnly);

    void MovingEmoteStopped();

    void execMovingEmoteStopped(FFrame& Stack, void* const Z_Param__Result);

    FMulticastScriptDelegate& GetOnWeaponUnEquippedDelegate();

    void SetLastReplicatedEmoteExecuted(UFortItemDefinition* InLastReplicatedEmoteExecuted);

    void OnRep_LastReplicatedEmoteExecuted();

    UFortHealthSet* GetHealthSet() const;

    void LaunchCharacterJump(const FVector& LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent);

    void SetGravityMultiplier(float NewGravityVal);
};
