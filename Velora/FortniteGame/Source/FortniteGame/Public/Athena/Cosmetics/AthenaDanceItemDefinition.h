#pragma once

#include "Items/FortMontageItemDefinitionBase.h"
#include "UObject/AssetPtr.h"

class UAthenaDanceItemDefinition : public UFortMontageItemDefinitionBase
{
public:
    static UClass* StaticClass();

    bool GetMovingEmote() const;

    bool GetMoveForwardOnly() const;

    bool GetMoveFollowingOnly() const;

    bool GetMovingEmoteSkipLandingFX() const;

    float GetWalkForwardSpeed() const;

    const TAssetPtr<UClass>* GetCustomDanceAbility() const;
};
