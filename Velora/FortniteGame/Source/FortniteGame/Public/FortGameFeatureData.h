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
    static UScriptStruct* StaticStruct();

    const TSoftObjectPtr<UDataTable>& GetLootTierData() const;

    const TSoftObjectPtr<UDataTable>& GetLootPackageData() const;
};

struct FFortGameFeatureDataBaseDataTableOverrides
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    const TSoftObjectPtr<UDataTable>& GetBaseTable() const;

    TArray<TPair<FGameplayTag, const TSoftObjectPtr<UDataTable>*>> GetPlaylistToOverrideTableMap() const;
};

class UFortGameFeatureData : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    const FFortGameFeatureLootTableData& GetDefaultLootTableData() const;

    TArray<TPair<FGameplayTag, const FFortGameFeatureLootTableData*>> GetPlaylistOverrideLootTableData() const;

    void GetLootTierDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& InOutLootTierDatas) const;

    void GetLootPackageDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& OutLootPackageDatas) const;

    const TRuntimeSizedArray<FFortGameFeatureDataBaseDataTableOverrides>* GetPlaylistBaseDataTableOverrides() const;

    static void GetActiveGameFeatureDatas(TArray<UFortGameFeatureData*>& OutGameFeatureDatas);

    static UDataTable* GetPlaylistDataTable(const UObject* WorldContextObject, UDataTable* BaseTable);
};
