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
#include "Items/FortWeaponItemDefinition.h"
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

FText UFortGameData::GetBuildingResourceName(EFortResourceType ResourceType) const
{
    static auto GetBuildingResourceName = []
    {
        auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

        return (FText* (*)(const UFortGameData*, FText*, EFortResourceType))ClientBuild.Find("call 0x*; movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*").Call(3).Get();
    }();

    FText Result(NoInit);
    GetBuildingResourceName(this, &Result, ResourceType);

    return Result;
}

UFortWeaponItemDefinition* UFortGameData::GetEditToolItem() const
{
    static auto EditToolItem = GetClass()->FindPropertyByName(FName(TEXT("EditToolItem")));

    return EditToolItem->ContainerPtrToValuePtr<TAssetPtr<UFortWeaponItemDefinition>>(this)->LoadSynchronous();
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
