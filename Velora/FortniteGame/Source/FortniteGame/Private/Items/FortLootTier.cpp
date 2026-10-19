#include "pch.h"
#include "Items/FortLootTier.h"
#include "FortGameData.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortLootTierData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortLootTierData"));

    return Struct;
}

bool UFortLootTier::ChooseUnquotaedLootTierKey(FName& OutLootTierKey, int32& OutLootTier, const FName& TierGroupName, int32 WorldLevel, const FGameplayTagContainer& GameplayTags, int32 ForcedLootTier)
{
    TArray<TPair<FName, FFortLootTierData*>> ValidRows;
    float TotalWeight = 0.0f;

    UFortGameData::Get()->FindLootTierDataByPredicate(ValidRows, TotalWeight, [&](const FFortLootTierData& LootTierData)
    {
        if (TierGroupName != LootTierData.TierGroup)
            return false;

        if (LootTierData.QuotaLevel != ELootQuotaLevel::Unlimited)
            return false;

        if (ForcedLootTier >= 0 && LootTierData.LootTier != ForcedLootTier)
            return false;

        if (LootTierData.RequiredGameplayTag.IsValid() && !GameplayTags.HasTag(LootTierData.RequiredGameplayTag))
            return false;

        if (WorldLevel >= 0)
        {
            if (LootTierData.MaxWorldLevel >= 0 && WorldLevel > LootTierData.MaxWorldLevel)
                return false;

            if (LootTierData.MinWorldLevel >= 0 && WorldLevel < LootTierData.MinWorldLevel)
                return false;
        }

        return true;
    });

    return PickLootTierKeyFromValidRows(OutLootTierKey, OutLootTier, TotalWeight, ValidRows);
}

bool UFortLootTier::PickLootTierKeyFromValidRows(FName& OutLootTierKey, int32& OutLootTier, float TotalWeight, const TArray<TPair<FName, FFortLootTierData*>>& ValidRows)
{
    if (FMath::Abs(TotalWeight) <= 1.e-8f)
        return false;

    float RandomWeight = FMath::FRand() * TotalWeight;

    for (auto& ValidRow : ValidRows)
    {
        if (RandomWeight <= ValidRow.Value->Weight)
        {
            OutLootTier = ValidRow.Value->LootTier;
            OutLootTierKey = ValidRow.Key;

            return true;
        }

        RandomWeight -= ValidRow.Value->Weight;
    }

    return false;
}

bool UFortLootTier::GetNumLootPackageDropsPerCategory(const FFortLootTierData* LootTierData, int32 NumLootPackageDrops, TMap<int32, int32>& OutNumLootPackageDropsPerCategory)
{
    OutNumLootPackageDropsPerCategory.Reset();

    if (!LootTierData || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num() || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryMaxArray.Num())
        return false;

    int32 NumDrops = 0;

    for (int32 Category = 0; Category < LootTierData->LootPackageCategoryMinArray.Num(); ++Category)
    {
        const int32 CategoryMin = LootTierData->LootPackageCategoryMinArray[Category];

        if (CategoryMin > 0)
        {
            OutNumLootPackageDropsPerCategory.Add(Category, CategoryMin);
            NumDrops += CategoryMin;
        }
    }

    if (NumDrops > NumLootPackageDrops)
    {
        UE_LOG(LogFort, Warning, TEXT("Requested %i loot drops but minimum drops is %i for loot package %s"), NumLootPackageDrops, NumDrops, *LootTierData->LootPackage.ToString());
    }

    TMap<int32, int32> CategoryWeights;
    int32 TotalWeight = 0;

    for (int32 Category = 0; Category < LootTierData->LootPackageCategoryWeightArray.Num(); ++Category)
    {
        const int32 CategoryWeight = LootTierData->LootPackageCategoryWeightArray[Category];

        if (CategoryWeight > 0)
        {
            const int32 CategoryMax = LootTierData->LootPackageCategoryMaxArray[Category];
            const int32* CategoryNum = OutNumLootPackageDropsPerCategory.Find(Category);

            if (CategoryMax < 0 || (CategoryNum ? *CategoryNum : 0) < CategoryMax)
            {
                TotalWeight += CategoryWeight;
                CategoryWeights.Add(Category, CategoryWeight);
            }
        }
    }

    if (NumDrops < NumLootPackageDrops)
    {
        while (TotalWeight > 0)
        {
            int32 RandomWeight = FMath::FloorToInt(TotalWeight * FMath::FRand());

            auto It = CategoryWeights.begin();

            for (; RandomWeight > It->Value; ++It)
            {
                RandomWeight -= It->Value;
            }

            const int32 Category = It->Key;
            const int32 CategoryWeight = It->Value;
            const int32 CategoryMax = LootTierData->LootPackageCategoryMaxArray[Category];

            int32& CategoryNum = OutNumLootPackageDropsPerCategory.FindOrAdd(Category);
            ++CategoryNum;
            ++NumDrops;

            if (CategoryMax >= 0)
            {
                if (CategoryNum > CategoryMax)
                {
                    UE_LOG(LogFort, Warning, TEXT("We have dropped %i items of category %i but the max is %i for tier group %s at tier %i."), CategoryNum, Category, CategoryMax, *LootTierData->TierGroup.ToString(), LootTierData->LootTier);
                }

                if (CategoryNum >= CategoryMax)
                {
                    TotalWeight -= CategoryWeight;
                    CategoryWeights.Remove(Category);
                }
            }

            if (NumDrops >= NumLootPackageDrops)
                break;
        }
    }

    return true;
}
