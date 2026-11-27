#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "UObject/AssetPtr.h"

class UFortAbilitySet;

class UGameDataBR : public UPrimaryDataAsset
{
public:
    static UGameDataBR* Get();

    UFortAbilitySet* GetPlayerAbilitySetBR() const;

    UObject* GetFortWeaponPickupSpawnAmmoData() const;

    const TArray<TAssetPtr<UDataTable>>& GetLootTierDataTablesBR() const;

    const TArray<TAssetPtr<UDataTable>>& GetLootPackageDataTablesBR() const;
};
