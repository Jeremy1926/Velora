#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

struct FGameplayTag
{
    FGameplayTag()
        : TagName(NAME_None)
    {
    }

    static FGameplayTag RequestGameplayTag(FName TagName, bool ErrorIfNotFound = true);

    FORCEINLINE bool operator==(FGameplayTag const& Other) const
    {
        return TagName == Other.TagName;
    }

    FORCEINLINE bool IsValid() const
    {
        return (TagName != NAME_None);
    }

    FORCEINLINE FName GetTagName() const
    {
        return TagName;
    }

    FORCEINLINE FString ToString() const
    {
        return TagName.ToString();
    }

protected:
    FName TagName;
};

struct FGameplayTagContainer
{
    FORCEINLINE bool HasTag(const FGameplayTag& TagToCheck) const
    {
        if (!TagToCheck.IsValid())
            return false;

        return GameplayTags.Contains(TagToCheck) || ParentTags.Contains(TagToCheck);
    }

    FORCEINLINE const TArray<FGameplayTag>& GetGameplayTagArray() const
    {
        return GameplayTags;
    }

    void AppendTags(FGameplayTagContainer const& Other);

    void AddTag(const FGameplayTag& TagToAdd);

    FString ToStringSimple(bool bQuoted = false) const;

protected:
    TArray<FGameplayTag> GameplayTags;
    TArray<FGameplayTag> ParentTags;
};
