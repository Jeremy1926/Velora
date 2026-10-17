#pragma once

#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/AssetPtr.h"
#include "GameplayTagContainer.h"

class UFortItemDefinition;
struct FFortItemEntry;

struct FFortLootPackageData : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    FName LootPackageID;
    float Weight;
    int32 Count;
    int32 LootPackageCategory;
    FName RequiredTag;
    FString LootPackageCall;
    TAssetPtr<UFortItemDefinition> ItemDefinition;
    FString PersistentLevel;
    int32 MinWorldLevel;
    int32 MaxWorldLevel;
    bool bAllowBonusDrops;
    FString Annotation;
};

class UFortLootPackage
{
public:
    static bool PickLootDrops(TArray<FFortItemEntry>& OutLootToDrop, int32 WorldLevel, FName LootTierKey, const FGameplayTagContainer& GameplayTags, bool bAllowBonusLootDrops, bool bUseLootPreviewPackage);

    static void PickLootDropsFromLootPackage(TArray<FFortItemEntry>& OutLootToDrop, FName LootPackageName, int32 WorldLevel, int32 LootPackageCategory, const FGameplayTagContainer& GameplayTags, bool bAllowBonusDrops);
};
