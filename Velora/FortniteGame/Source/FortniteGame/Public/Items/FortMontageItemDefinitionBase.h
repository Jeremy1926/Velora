#pragma once

#include "Items/FortAccountItemDefinition.h"
#include "UObject/AssetPtr.h"

class UFortMontageItemDefinitionBase : public UFortAccountItemDefinition
{
public:
    const TAssetPtr<UObject>& GetAnimation() const;
};
