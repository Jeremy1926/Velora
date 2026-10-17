#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager : public UObject
{
public:
    static UGameplayTagsManager& Get();

    static void InitializeManager();

    FGameplayTag RequestGameplayTag(FName TagName, bool ErrorIfNotFound = true) const;
};
