#pragma once

#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"

enum class ELootQuotaLevel : uint8
{
    Unlimited,
    Level1,
    Level2,
    Level3,
    Level4,
    Level5,
    Level6,
    Level7,
    Level8,
    Level9,
    Level10,
    Level11,
    Level12,
    Level13,
    Level14,
    Level15,
    Level16,
    Level17,
    NumLevels,
};

struct FFortLootTierData : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    FName TierGroup;
    float Weight;
    ELootQuotaLevel QuotaLevel;
    int32 LootTier;
    int32 MinWorldLevel;
    int32 MaxWorldLevel;
    FString StreakBreakerCurrency;
    int32 StreakBreakerPointsMin;
    int32 StreakBreakerPointsMax;
    int32 StreakBreakerPointsSpend;
    FName LootPackage;
    FName LootPreviewPackage;
    float NumLootPackageDrops;
    TArray<int32> LootPackageCategoryWeightArray;
    TArray<int32> LootPackageCategoryMinArray;
    TArray<int32> LootPackageCategoryMaxArray;

    FGameplayTag GetRequiredGameplayTag() const;

    bool GetAllowBonusLootDrops() const;
};

class UFortLootTier
{
public:
    static bool ChooseUnquotaedLootTierKey(FName& OutLootTierKey, int32& OutLootTier, const FName& TierGroupName, int32 WorldLevel, const FGameplayTagContainer& GameplayTags, int32 ForcedLootTier = -1);

    static bool PickLootTierKeyFromValidRows(FName& OutLootTierKey, int32& OutLootTier, float TotalWeight, const TArray<TPair<FName, FFortLootTierData*>>& ValidRows);

    static bool GetNumLootPackageDropsPerCategory(const FFortLootTierData* LootTierData, int32 NumLootPackageDrops, TMap<int32, int32>& OutNumLootPackageDropsPerCategory);
};
