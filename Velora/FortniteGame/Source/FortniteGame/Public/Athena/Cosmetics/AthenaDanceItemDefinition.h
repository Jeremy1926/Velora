#pragma once

#include "Items/FortMontageItemDefinitionBase.h"
#include "UObject/AssetPtr.h"

class UAthenaDanceItemDefinition : public UFortMontageItemDefinitionBase
{
public:
    static UClass* StaticClass();

    bool GetMovingEmote() const;

    bool GetMoveForwardOnly() const;

    const TAssetPtr<UClass>* GetCustomDanceAbility() const;
};
