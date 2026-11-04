#include "pch.h"
#include "FortGameFeatureData.h"
#include "FortGameFeaturePluginManager.h"
#include "GameFeaturesSubsystem.h"
#include "Athena/FortGameStateAthena.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortGameFeatureLootTableData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameFeatureLootTableData"));

    return Struct;
}

const TSoftObjectPtr<UDataTable>& FFortGameFeatureLootTableData::GetLootTierData() const
{
    static auto LootTierData = StaticStruct()->FindPropertyByName(FName(TEXT("LootTierData")));

    return *LootTierData->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

const TSoftObjectPtr<UDataTable>& FFortGameFeatureLootTableData::GetLootPackageData() const
{
    static auto LootPackageData = StaticStruct()->FindPropertyByName(FName(TEXT("LootPackageData")));

    return *LootPackageData->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

UScriptStruct* FFortGameFeatureDataBaseDataTableOverrides::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameFeatureDataBaseDataTableOverrides"));

    return Struct;
}

int32 FFortGameFeatureDataBaseDataTableOverrides::GetTypeSize()
{
    return StaticStruct()->GetPropertiesSize();
}

const TSoftObjectPtr<UDataTable>& FFortGameFeatureDataBaseDataTableOverrides::GetBaseTable() const
{
    static auto BaseTable = StaticStruct()->FindPropertyByName(FName(TEXT("BaseTable")));

    return *BaseTable->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

TArray<TPair<FGameplayTag, const TSoftObjectPtr<UDataTable>*>> FFortGameFeatureDataBaseDataTableOverrides::GetPlaylistToOverrideTableMap() const
{
    static auto PlaylistToOverrideTableMap = StaticStruct()->FindPropertyByName(FName(TEXT("PlaylistToOverrideTableMap")));
    static auto ValueAlignment = (int32)alignof(FSoftObjectPtr);
    static auto PairAlignment = FMath::Max(ValueAlignment, (int32)sizeof(int32));
    static auto ValueOffset = (FGameplayTag::GetTypeSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + TSoftObjectPtr<UDataTable>::GetTypeSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    static auto ElementSize = FCompactSetBase::IsEnabled() ? PairSize : (PairSize + 2 * (int32)sizeof(int32) + PairAlignment - 1) & ~(PairAlignment - 1);

    auto& Elements = *PlaylistToOverrideTableMap->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);
    auto ElementData = *PlaylistToOverrideTableMap->ContainerPtrToValuePtr<const uint8*>(this);
    TArray<TPair<FGameplayTag, const TSoftObjectPtr<UDataTable>*>> Pairs;

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        if (FCompactSetBase::IsEnabled() || Elements.IsAllocated(Index))
            Pairs.Add({ TGameLayout<FGameplayTag>::Load(ElementData + Index * ElementSize), (const TSoftObjectPtr<UDataTable>*)(ElementData + Index * ElementSize + ValueOffset) });
    }

    return Pairs;
}

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

TArray<TPair<FGameplayTag, const FFortGameFeatureLootTableData*>> UFortGameFeatureData::GetPlaylistOverrideLootTableData() const
{
    static auto PlaylistOverrideLootTableData = StaticClass()->FindPropertyByName(FName(TEXT("PlaylistOverrideLootTableData")));
    static auto ValueAlignment = FFortGameFeatureLootTableData::StaticStruct()->GetMinAlignment();
    static auto PairAlignment = FMath::Max(ValueAlignment, (int32)sizeof(int32));
    static auto ValueOffset = (FGameplayTag::GetTypeSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + FFortGameFeatureLootTableData::StaticStruct()->GetPropertiesSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    static auto ElementSize = FCompactSetBase::IsEnabled() ? PairSize : (PairSize + 2 * (int32)sizeof(int32) + PairAlignment - 1) & ~(PairAlignment - 1);

    auto& Elements = *PlaylistOverrideLootTableData->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);
    auto ElementData = *PlaylistOverrideLootTableData->ContainerPtrToValuePtr<const uint8*>(this);
    TArray<TPair<FGameplayTag, const FFortGameFeatureLootTableData*>> Pairs;

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        if (FCompactSetBase::IsEnabled() || Elements.IsAllocated(Index))
            Pairs.Add({ TGameLayout<FGameplayTag>::Load(ElementData + Index * ElementSize), (const FFortGameFeatureLootTableData*)(ElementData + Index * ElementSize + ValueOffset) });
    }

    return Pairs;
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
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value->GetLootTierData().IsNull())
        {
            AddLootTable(PlaylistOverride.Value->GetLootTierData());
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().GetLootTierData().IsNull())
        AddLootTable(GetDefaultLootTableData().GetLootTierData());
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
        if (PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key) && !PlaylistOverride.Value->GetLootPackageData().IsNull())
        {
            AddLootTable(PlaylistOverride.Value->GetLootPackageData());
            bFoundOverride = true;
        }
    }

    if (!bFoundOverride && !GetDefaultLootTableData().GetLootPackageData().IsNull())
        AddLootTable(GetDefaultLootTableData().GetLootPackageData());
}

const TRuntimeSizedArray<FFortGameFeatureDataBaseDataTableOverrides>* UFortGameFeatureData::GetPlaylistBaseDataTableOverrides() const
{
    static auto PlaylistBaseDataTableOverrides = StaticClass()->FindPropertyByName(FName(TEXT("PlaylistBaseDataTableOverrides")));

    return PlaylistBaseDataTableOverrides ? PlaylistBaseDataTableOverrides->ContainerPtrToValuePtr<TRuntimeSizedArray<FFortGameFeatureDataBaseDataTableOverrides>>(this) : nullptr;
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
            if (Override.GetBaseTable().LoadSynchronous() != BaseTable)
                continue;

            for (auto& PlaylistOverride : Override.GetPlaylistToOverrideTableMap())
            {
                if (!PlaylistTags.GetGameplayTagArray().Contains(PlaylistOverride.Key))
                    continue;

                if (auto OverrideTable = PlaylistOverride.Value->LoadSynchronous())
                    return OverrideTable;
            }
        }
    }

    return BaseTable;
}
