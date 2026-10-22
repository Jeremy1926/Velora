#include "pch.h"
#include "Items/FortCampsiteAccountItem.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "FortAssetManager.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FCachedItemData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.CachedItemData"));

    return Struct;
}

int32 FCachedItemData::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

const FString& FCachedItemData::GetItemDefinitionPersistentName() const
{
    static auto ItemDefinitionPersistentName = StaticStruct()->FindPropertyByName(FName(TEXT("item_definition_persistent_name")));

    return *ItemDefinitionPersistentName->ContainerPtrToValuePtr<FString>(this);
}

int32 FCachedItemData::GetItemCount() const
{
    static auto ItemCount = StaticStruct()->FindPropertyByName(FName(TEXT("item_count")));

    return *ItemCount->ContainerPtrToValuePtr<int32>(this);
}

int32 FCachedItemData::GetItemLevel() const
{
    static auto ItemLevel = StaticStruct()->FindPropertyByName(FName(TEXT("item_level")));

    return *ItemLevel->ContainerPtrToValuePtr<int32>(this);
}

int32 FCachedItemData::GetItemLoadedAmmo() const
{
    static auto ItemLoadedAmmo = StaticStruct()->FindPropertyByName(FName(TEXT("item_loaded_ammo")));

    return *ItemLoadedAmmo->ContainerPtrToValuePtr<int32>(this);
}

UScriptStruct* FCampsiteAccountData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.CampsiteAccountData"));

    return Struct;
}

TRuntimeSizedArray<FCachedItemData>& FCampsiteAccountData::GetStashedItems()
{
    static auto StashedItems = StaticStruct()->FindPropertyByName(FName(TEXT("stashed_items")));

    return *StashedItems->ContainerPtrToValuePtr<TRuntimeSizedArray<FCachedItemData>>(this);
}

void FCampsiteAccountData::CopyFromMCPData(TArray<FFortItemEntry>& ItemEntriesOut)
{
    static auto OnlyRetrieveItemIfLoaded = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.OnlyRetrieveItemIfLoaded"));

    auto& StashedItems = GetStashedItems();

    for (int32 Index = 0; Index < StashedItems.Num() && Index < ItemEntriesOut.Num(); Index++)
    {
        auto& CachedItemData = StashedItems[Index];
        auto& PersistentName = CachedItemData.GetItemDefinitionPersistentName();

        if (PersistentName.IsEmpty())
            continue;

        auto ItemDefinition = Cast<UFortItemDefinition>(OnlyRetrieveItemIfLoaded && OnlyRetrieveItemIfLoaded->GetInt() ? UKismetSystemLibrary::GetObjectFromPrimaryAssetId(FPrimaryAssetId::ParseTypeAndName(PersistentName)) : UFortAssetManager::Get().GetAssetForPersistentName(PersistentName));

        if (!ItemDefinition)
            continue;

        ItemEntriesOut[Index].~FFortItemEntry();
        new (&ItemEntriesOut[Index]) FFortItemEntry(ItemDefinition, CachedItemData.GetItemCount(), CachedItemData.GetItemLevel());
        ItemEntriesOut[Index].SetLoadedAmmo(CachedItemData.GetItemLoadedAmmo());
    }
}

UClass* UFortCampsiteAccountItem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortCampsiteAccountItem"));

    return Class;
}

FCampsiteAccountData& UFortCampsiteAccountItem::GetCampsiteAccountData()
{
    static auto CampsiteAccountData = StaticClass()->FindPropertyByName(FName(TEXT("campsite_account_data")));

    return *CampsiteAccountData->ContainerPtrToValuePtr<FCampsiteAccountData>(this);
}
