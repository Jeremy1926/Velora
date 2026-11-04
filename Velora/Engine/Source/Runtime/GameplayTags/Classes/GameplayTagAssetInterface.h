#pragma once

struct FGameplayTag;
struct FGameplayTagContainer;

class IGameplayTagAssetInterface
{
public:
    void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

    bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const;
};
