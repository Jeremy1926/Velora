#pragma once

#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/WeakObjectPtr.h"

class AFortWeapon;
class UFortWeaponItemDefinition;
class UFortWorldItem;
struct FFrame;

enum class EFortGameplayAbilityTagChangedActionType : uint8
{
    Cancel,
};

enum class EFortGameplayAbilityTagChangedEvent : uint8
{
    NewlyAdded,
    Increased,
    FullyRemoved,
    Decreased,
};

class UFortGameplayAbility : public UGameplayAbility
{
public:
    static UClass* StaticClass();

    void TagChangedActionCallback(const FGameplayTag Tag, bool bTagExists);

    void EquipAbilityWeapon(UFortWeaponItemDefinition* WeaponDefinition, int32 ItemLevel);

    void execEquipAbilityWeapon(FFrame& Stack, void* const Z_Param__Result);

    TWeakObjectPtr<UFortWorldItem>& GetAbilityWeaponItem();

    TWeakObjectPtr<AFortWeapon>& GetAbilityWeapon();

    TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectContainer(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag, int32 GameplayEffectLevel);

    float GetCurrentAbilitySourceLevel();

    FGameplayTag GetEffectContainerApplicationTag() const;
};
