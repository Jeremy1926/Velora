#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

class UDataTable;

struct FFortGameFeatureLootTableData
{
    TSoftObjectPtr<UDataTable> LootTierData;
    TSoftObjectPtr<UDataTable> LootPackageData;
};

class UFortGameFeatureData : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    const FFortGameFeatureLootTableData& GetDefaultLootTableData() const;

    const TMap<FGameplayTag, FFortGameFeatureLootTableData>& GetPlaylistOverrideLootTableData() const;

    void GetLootTierDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& InOutLootTierDatas) const;

    void GetLootPackageDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& OutLootPackageDatas) const;
};
