#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"
#include "Templates/TypeHash.h"
#include "Engine/NetSerialization.h"
#include "GameplayEffectTypes.h"
#include "GameplayPrediction.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UObject;

struct FGameplayAbilitySpecHandle
{
    FGameplayAbilitySpecHandle()
        : Handle(INDEX_NONE)
    {
    }

    bool IsValid() const
    {
        return Handle != INDEX_NONE;
    }

    void GenerateNewHandle();

    bool operator==(const FGameplayAbilitySpecHandle& Other) const
    {
        return Handle == Other.Handle;
    }

    bool operator!=(const FGameplayAbilitySpecHandle& Other) const
    {
        return Handle != Other.Handle;
    }

    friend uint32 GetTypeHash(const FGameplayAbilitySpecHandle& SpecHandle)
    {
        return ::GetTypeHash(SpecHandle.Handle);
    }

private:
    int32 Handle;
};

namespace EGameplayAbilityActivationMode
{
    enum Type
    {
        Authority,
        NonAuthority,
        Predicting,
        Confirmed,
        Rejected,
    };
}

struct FGameplayAbilityActivationInfo
{
    FGameplayAbilityActivationInfo()
        : ActivationMode(EGameplayAbilityActivationMode::Authority)
        , bCanBeEndedByOtherInstance(false)
    {
    }

    mutable TEnumAsByte<EGameplayAbilityActivationMode::Type> ActivationMode;

    uint8 bCanBeEndedByOtherInstance : 1;

private:
    FPredictionKey PredictionKeyWhenActivated;
};

struct FGameplayAbilitySpec : public FFastArraySerializerItem
{
    FGameplayAbilitySpec()
        : Ability(nullptr), Level(1), InputID(INDEX_NONE), SourceObject(nullptr), ActiveCount(0), InputPressed(false), RemoveAfterActivation(false), PendingRemove(false)
    {
    }

    FGameplayAbilitySpec(UGameplayAbility* InAbility, int32 InLevel = 1, int32 InInputID = INDEX_NONE, UObject* InSourceObject = nullptr)
        : Ability(InAbility), Level(InLevel), InputID(InInputID), SourceObject(InSourceObject), ActiveCount(0), InputPressed(false), RemoveAfterActivation(false), PendingRemove(false)
    {
        Handle.GenerateNewHandle();
    }

    FGameplayAbilitySpecHandle Handle;

    UGameplayAbility* Ability;

    int32 Level;

    int32 InputID;

    UObject* SourceObject;

    uint8 ActiveCount;

    uint8 InputPressed : 1;

    uint8 RemoveAfterActivation : 1;

    uint8 PendingRemove : 1;

    FGameplayAbilityActivationInfo ActivationInfo;

    TArray<UGameplayAbility*> NonReplicatedInstances;

    TArray<UGameplayAbility*> ReplicatedInstances;

    FActiveGameplayEffectHandle GameplayEffectHandle;
};

struct FGameplayAbilitySpecContainer : public FFastArraySerializer
{
    TArray<FGameplayAbilitySpec> Items;

    UAbilitySystemComponent* Owner;
};
