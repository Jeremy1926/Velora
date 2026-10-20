#pragma once

struct FGameplayTagContainer;

class IGameplayTagAssetInterface
{
public:
    void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;
};
