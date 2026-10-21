#include "pch.h"
#include "FortGameFeatureData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortGameFeatureData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameFeatureData"));

    return Class;
}

const FFortGameFeatureLootTableData& UFortGameFeatureData::GetDefaultLootTableData() const
{
    static auto DefaultLootTableData = StaticClass()->FindPropertyByName(FName(TEXT("DefaultLootTableData")));

    return *DefaultLootTableData->ContainerPtrToValuePtr<FFortGameFeatureLootTableData>(this);
}

const TMap<FGameplayTag, FFortGameFeatureLootTableData>& UFortGameFeatureData::GetPlaylistOverrideLootTableData() const
{
    static auto PlaylistOverrideLootTableData = StaticClass()->FindPropertyByName(FName(TEXT("PlaylistOverrideLootTableData")));

    return *PlaylistOverrideLootTableData->ContainerPtrToValuePtr<TMap<FGameplayTag, FFortGameFeatureLootTableData>>(this);
}

void UFortGameFeatureData::GetLootTierDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& InOutLootTierDatas) const
{
    bool bFoundOverride = false;

    for (auto& PlaylistOverride : GetPlaylistOverrideLootTableData())
    {
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value.LootTierData.IsNull())
        {
            InOutLootTierDatas.Add(PlaylistOverride.Value.LootTierData);
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().LootTierData.IsNull())
        InOutLootTierDatas.Add(GetDefaultLootTableData().LootTierData);
}

void UFortGameFeatureData::GetLootPackageDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& OutLootPackageDatas) const
{
    bool bFoundOverride = false;

    for (auto& PlaylistOverride : GetPlaylistOverrideLootTableData())
    {
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value.LootPackageData.IsNull())
        {
            OutLootPackageDatas.Add(PlaylistOverride.Value.LootPackageData);
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().LootPackageData.IsNull())
        OutLootPackageDatas.Add(GetDefaultLootTableData().LootPackageData);
}
