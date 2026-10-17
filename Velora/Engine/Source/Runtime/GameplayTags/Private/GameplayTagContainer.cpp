#include "pch.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"

FGameplayTag FGameplayTag::RequestGameplayTag(FName TagName, bool ErrorIfNotFound)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(TagName, ErrorIfNotFound);
}

FString FGameplayTagContainer::ToStringSimple(bool bQuoted) const
{
    FString RetString;

    for (int i = 0; i < GameplayTags.Num(); ++i)
    {
        if (bQuoted)
        {
            RetString += TEXT("\"");
        }

        RetString += GameplayTags[i].ToString();

        if (bQuoted)
        {
            RetString += TEXT("\"");
        }

        if (i < GameplayTags.Num() - 1)
        {
            RetString += TEXT(", ");
        }
    }

    return RetString;
}

void FGameplayTagContainer::AppendTags(FGameplayTagContainer const& Other)
{
    for (const FGameplayTag& OtherTag : Other.GameplayTags)
    {
        if (!GameplayTags.Contains(OtherTag))
            GameplayTags.Add(OtherTag);
    }

    for (const FGameplayTag& OtherTag : Other.ParentTags)
    {
        if (!ParentTags.Contains(OtherTag))
            ParentTags.Add(OtherTag);
    }
}
