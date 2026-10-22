#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

class UDataTable;
class UScriptStruct;

struct FFortGameFeatureLootTableData
{
    TSoftObjectPtr<UDataTable> LootTierData;
    TSoftObjectPtr<UDataTable> LootPackageData;
};

struct FFortGameFeatureDataBaseDataTableOverrides
{
    TSoftObjectPtr<UDataTable> BaseTable;
    TMap<FGameplayTag, TSoftObjectPtr<UDataTable>> PlaylistToOverrideTableMap;
};

class UFortGameFeatureData : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    const FFortGameFeatureLootTableData& GetDefaultLootTableData() const;

    const TMap<FGameplayTag, FFortGameFeatureLootTableData>& GetPlaylistOverrideLootTableData() const;

    void GetLootTierDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& InOutLootTierDatas) const;

    void GetLootPackageDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& OutLootPackageDatas) const;

    const TArray<FFortGameFeatureDataBaseDataTableOverrides>* GetPlaylistBaseDataTableOverrides() const;

    static void GetActiveGameFeatureDatas(TArray<UFortGameFeatureData*>& OutGameFeatureDatas);

    static UDataTable* GetPlaylistDataTable(const UObject* WorldContextObject, UDataTable* BaseTable);

    static UDataTable* FindPlaylistDataTable(const UObject* WorldContextObject, const UScriptStruct* RowStruct);
};
