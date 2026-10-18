#include "pch.h"
#include "FortGameData.h"
#include "FortAssetManager.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/AssetPtr.h"
#include "UObject/UnrealType.h"
#include "Items/FortLootTier.h"
#include "Items/FortLootPackage.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Items/FortResourceItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortGameData* UFortGameData::Get()
{
    return UFortAssetManager::Get().GetGameData();
}

UFortAbilitySet* UFortGameData::GetGenericPlayerAbilitySet() const
{
    static auto GenericPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("GenericPlayerAbilitySet")));

    return GenericPlayerAbilitySet->ContainerPtrToValuePtr<TAssetPtr<UFortAbilitySet>>(this)->LoadSynchronous();
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootTierDataTables() const
{
    static auto LootTierDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootTierDataTables")));

    return *LootTierDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootPackageDataTables() const
{
    static auto LootPackageDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootPackageDataTables")));

    return *LootPackageDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}

FFortLootTierData* UFortGameData::FindLootTierDataRow(FName TierGroupName, const FString& ContextString) const
{
    for (auto& LootTierDataTable : GetLootTierDataTables())
    {
        if (auto DataTable = LootTierDataTable.LoadSynchronous())
        {
            if (auto LootTierData = DataTable->FindRow<FFortLootTierData>(TierGroupName, *ContextString, false))
                return LootTierData;
        }
    }

    UE_LOG(LogFort, Warning, TEXT("UFortGameData::FindLootTierDataRow : TierGroupName %s with context %s not found!"), *TierGroupName.ToString(), *ContextString);

    return nullptr;
}

bool UFortGameData::HasLootPackageData() const
{
    for (auto& LootPackageDataTable : GetLootPackageDataTables())
    {
        if (LootPackageDataTable.LoadSynchronous())
            return true;
    }

    return false;
}

UFortResourceItemDefinition* UFortGameData::GetResourceItemDefinition(EFortResourceType ResourceType) const
{
    static auto GetResourceItemDefinition = (UFortResourceItemDefinition* (*)(const UFortGameData*, EFortResourceType))Scanner::Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition")).Call(2).Get();

    return GetResourceItemDefinition(this, ResourceType);
}
