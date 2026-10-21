#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/EnumAsByte.h"
#include "Templates/TypeHash.h"
#include "Engine/NetSerialization.h"
#include "GameplayEffectTypes.h"
#include "GameplayPrediction.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UObject;
class UScriptStruct;

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

struct alignas(8) FGameplayAbilitySpec : public FFastArraySerializerItem
{
    FGameplayAbilitySpec(UGameplayAbility* InAbility, int32 InLevel = 1, int32 InInputID = INDEX_NONE, UObject* InSourceObject = nullptr);

    FGameplayAbilitySpec(const FGameplayAbilitySpec&) = delete;

    ~FGameplayAbilitySpec();

    FGameplayAbilitySpecHandle Handle;

    UGameplayAbility* Ability;

    int32 Level;

    int32 InputID;

    UObject* SourceObject;

    uint8 ActiveCount;

    uint8 InputPressed : 1;

    uint8 RemoveAfterActivation : 1;

    uint8 PendingRemove : 1;

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

private:
    uint8 Data[0xD6];
};

struct FGameplayAbilitySpecContainer : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    TRuntimeSizedArray<FGameplayAbilitySpec>& GetItems();
};
