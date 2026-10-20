#pragma once

#include "Engine/DataAsset.h"
#include "UObject/AssetPtr.h"

class UGameDataCosmetics : public UPrimaryDataAsset
{
public:
    static UGameDataCosmetics* Get();

    const TAssetPtr<UClass>& GetEmoteGameplayAbility() const;
};
