#include "pch.h"
#include "FortGameFeatureData.h"
#include "FortGameFeaturePluginManager.h"
#include "GameFeaturesSubsystem.h"
#include "Athena/FortGameStateAthena.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
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
    auto AddLootTable = [&InOutLootTierDatas](const TSoftObjectPtr<UDataTable>& LootTable)
    {
        auto Table = LootTable.LoadSynchronous();

        for (auto& ExistingLootTable : InOutLootTierDatas)
        {
            if (ExistingLootTable.LoadSynchronous() == Table)
                return;
        }

        InOutLootTierDatas.Add(LootTable);
    };

    bool bFoundOverride = false;

    for (auto& PlaylistOverride : GetPlaylistOverrideLootTableData())
    {
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value.LootTierData.IsNull())
        {
            AddLootTable(PlaylistOverride.Value.LootTierData);
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().LootTierData.IsNull())
        AddLootTable(GetDefaultLootTableData().LootTierData);
}

void UFortGameFeatureData::GetLootPackageDataForPlaylist(const FGameplayTagContainer& PlaylistTags, TArray<TSoftObjectPtr<UDataTable>>& OutLootPackageDatas) const
{
    auto AddLootTable = [&OutLootPackageDatas](const TSoftObjectPtr<UDataTable>& LootTable)
    {
        auto Table = LootTable.LoadSynchronous();

        for (auto& ExistingLootTable : OutLootPackageDatas)
        {
            if (ExistingLootTable.LoadSynchronous() == Table)
                return;
        }

        OutLootPackageDatas.Add(LootTable);
    };

    bool bFoundOverride = false;

    for (auto& PlaylistOverride : GetPlaylistOverrideLootTableData())
    {
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value.LootPackageData.IsNull())
        {
            AddLootTable(PlaylistOverride.Value.LootPackageData);
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().LootPackageData.IsNull())
        AddLootTable(GetDefaultLootTableData().LootPackageData);
}

const TArray<FFortGameFeatureDataBaseDataTableOverrides>* UFortGameFeatureData::GetPlaylistBaseDataTableOverrides() const
{
    static auto PlaylistBaseDataTableOverrides = StaticClass()->FindPropertyByName(FName(TEXT("PlaylistBaseDataTableOverrides")));

    return PlaylistBaseDataTableOverrides ? PlaylistBaseDataTableOverrides->ContainerPtrToValuePtr<TArray<FFortGameFeatureDataBaseDataTableOverrides>>(this) : nullptr;
}

void UFortGameFeatureData::GetActiveGameFeatureDatas(TArray<UFortGameFeatureData*>& OutGameFeatureDatas)
{
    if (auto GameFeaturePluginManager = UFortGameFeaturePluginManager::Get())
    {
        GameFeaturePluginManager->GetGameFeatureDataForActivePlugins(OutGameFeatureDatas);
        return;
    }

    auto GameFeaturesSubsystem = UGameFeaturesSubsystem::Get();

    if (!GameFeaturesSubsystem)
        return;

    TArray<const UGameFeatureData*> ActivePluginFeatureDatas;
    GameFeaturesSubsystem->GetGameFeatureDataForActivePlugins(ActivePluginFeatureDatas);

    for (auto GameFeatureData : ActivePluginFeatureDatas)
    {
        if (auto FortGameFeatureData = Cast<UFortGameFeatureData>((UObject*)GameFeatureData))
            OutGameFeatureDatas.Add(FortGameFeatureData);
    }
}

UDataTable* UFortGameFeatureData::GetPlaylistDataTable(const UObject* WorldContextObject, UDataTable* BaseTable)
{
    auto World = WorldContextObject && BaseTable ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;

    if (!GameState)
        return BaseTable;

    const FGameplayTagContainer PlaylistTags = GameState->GetAthenaPlaylistContextTags();
    TArray<UFortGameFeatureData*> GameFeatureDatas;
    GetActiveGameFeatureDatas(GameFeatureDatas);

    for (auto GameFeatureData : GameFeatureDatas)
    {
        auto Overrides = GameFeatureData ? GameFeatureData->GetPlaylistBaseDataTableOverrides() : nullptr;

        if (!Overrides)
            continue;

        for (auto& Override : *Overrides)
        {
            if (Override.BaseTable.LoadSynchronous() != BaseTable)
                continue;

            for (auto& PlaylistOverride : Override.PlaylistToOverrideTableMap)
            {
                if (!PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key))
                    continue;

                if (auto OverrideTable = PlaylistOverride.Value.LoadSynchronous())
                    return OverrideTable;
            }
        }
    }

    return BaseTable;
}
