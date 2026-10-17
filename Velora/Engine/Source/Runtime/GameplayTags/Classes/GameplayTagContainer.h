#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

struct FGameplayTag
{
    FORCEINLINE FString ToString() const
    {
        return TagName.ToString();
    }

protected:
    FName TagName;
};

struct FGameplayTagContainer
{
    FString ToStringSimple(bool bQuoted = false) const;

protected:
    TArray<FGameplayTag> GameplayTags;
    TArray<FGameplayTag> ParentTags;
};
