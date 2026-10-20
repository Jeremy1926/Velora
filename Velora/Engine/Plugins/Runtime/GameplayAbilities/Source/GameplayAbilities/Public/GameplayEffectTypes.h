#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "GameplayTagContainer.h"

struct FGameplayEffectContext;
struct FGameplayEffectSpec;

struct FActiveGameplayEffectHandle
{
    FActiveGameplayEffectHandle()
        : Handle(INDEX_NONE)
        , bPassedFiltersAndWasExecuted(false)
    {
    }

private:
    int32 Handle;
    bool bPassedFiltersAndWasExecuted;
};

struct FGameplayEffectContextHandle
{
    FGameplayEffectContextHandle()
    {
    }

    FGameplayEffectContextHandle(FGameplayEffectContextHandle&& Other) = default;

    virtual ~FGameplayEffectContextHandle()
    {
    }

private:
    TSharedPtr<FGameplayEffectContext> Data;
};

struct FGameplayEffectSpecHandle
{
    FGameplayEffectSpecHandle()
        : IsValidCache(false)
    {
    }

    bool IsValid() const
    {
        return Data.IsValid();
    }

    TSharedPtr<FGameplayEffectSpec> Data;

    bool IsValidCache;
};

struct FScalableFloat
{
    float GetValueAtLevel(float Level, const FString* ContextString = nullptr) const;
};

struct FGameplayTagCountContainer
{
    FORCEINLINE bool UpdateTagCount(const FGameplayTag& Tag, int32 CountDelta)
    {
        if (CountDelta != 0)
        {
            return UpdateTagMap_Internal(Tag, CountDelta);
        }

        return false;
    }

private:
    bool UpdateTagMap_Internal(const FGameplayTag& Tag, int32 CountDelta, bool bDeferParentTagsOnRemove = false);
};
