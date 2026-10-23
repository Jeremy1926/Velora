#pragma once

#include "GameFramework/Character.h"
#include "Math/Vector.h"
#include "Misc/Guid.h"
#include "UObject/ScriptDelegates.h"
#include "Delegates/Delegate.h"

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

    static TMulticastDelegate<void, AFortPawn*, AFortWeapon*, AFortWeapon*>& GetNotifyWeaponChange();

    AFortWeapon* GetCurrentWeapon() const;

    AFortWeapon* EquipWeaponDefinition(const UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid, const FGuid& TrackerGuid = FGuid());

    bool EquipBestWeapon();

    UAbilitySystemComponent* GetAbilitySystemComponent() const;

    void StopPerformingNativeAction(const FGameplayTag& ActionTag);

    bool GetIsDBNO() const;

    void SetIsDBNO(bool bInIsDBNO);

    bool GetWasDBNOOnDeath() const;

    void SetPlayedDying(bool bInPlayedDying);

    void SetMovingEmote(bool bInMovingEmote);

    void SetMovingEmoteForwardOnly(bool bInMovingEmoteForwardOnly);

    void SetMovingEmoteFollowingOnly(bool bInMovingEmoteFollowingOnly);

    void SetMovingEmoteSkipLandingFX(bool bInMovingEmoteSkipLandingFX);

    void SetEmoteWalkSpeed(float InEmoteWalkSpeed);

    void MovingEmoteStopped();

    void execMovingEmoteStopped(FFrame& Stack, void* const Z_Param__Result);

    FMulticastScriptDelegate& GetOnWeaponUnEquippedDelegate();

    UFortItemDefinition* GetLastReplicatedEmoteExecuted() const;

    void SetLastReplicatedEmoteExecuted(UFortItemDefinition* InLastReplicatedEmoteExecuted);

    void OnRep_LastReplicatedEmoteExecuted(const UFortItemDefinition* PreviousValue = nullptr);

    UFortHealthSet* GetHealthSet() const;

    void LaunchCharacterJump(const FVector& LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent);

    void SetGravityMultiplier(float NewGravityVal);

    void Suicide();

    TArray<AFortWeapon*>& GetCurrentWeaponList();

    void UpdateCullDistance();

    bool UpdateSharedReplication();
};
