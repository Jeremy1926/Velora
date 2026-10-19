#include "pch.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortLootLevel.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortGameData.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortLootPackageData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortLootPackageData"));

    return Struct;
}

FName FFortLootPackageData::GetRequiredTag() const
{
    static auto RequiredTag = StaticStruct()->FindPropertyByName(FName(TEXT("RequiredTag")));

    return *RequiredTag->ContainerPtrToValuePtr<FName>(this);
}

const FString& FFortLootPackageData::GetLootPackageCall() const
{
    static auto LootPackageCall = StaticStruct()->FindPropertyByName(FName(TEXT("LootPackageCall")));

    return *LootPackageCall->ContainerPtrToValuePtr<FString>(this);
}

const TAssetPtr<UFortItemDefinition>& FFortLootPackageData::GetItemDefinition() const
{
    static auto ItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("ItemDefinition")));

    return *ItemDefinition->ContainerPtrToValuePtr<TAssetPtr<UFortItemDefinition>>(this);
}

int32 FFortLootPackageData::GetMinWorldLevel() const
{
    static auto MinWorldLevel = StaticStruct()->FindPropertyByName(FName(TEXT("MinWorldLevel")));

    return *MinWorldLevel->ContainerPtrToValuePtr<int32>(this);
}

int32 FFortLootPackageData::GetMaxWorldLevel() const
{
    static auto MaxWorldLevel = StaticStruct()->FindPropertyByName(FName(TEXT("MaxWorldLevel")));

    return *MaxWorldLevel->ContainerPtrToValuePtr<int32>(this);
}

static bool PackageMeetsCriteria(const FFortLootPackageData& LootPackageData, FName LootPackageName, int32 LootPackageCategory, const FGameplayTagContainer& GameplayTags, int32 WorldLevel)
{
    if (LootPackageName != LootPackageData.LootPackageID || (LootPackageCategory != -1 && LootPackageData.LootPackageCategory != LootPackageCategory))
        return false;

    if (LootPackageData.GetRequiredTag() != NAME_None)
    {
        if (!GameplayTags.HasTag(FGameplayTag::RequestGameplayTag(LootPackageData.GetRequiredTag())))
            return false;
    }

    if (WorldLevel >= 0)
    {
        if (LootPackageData.GetMaxWorldLevel() >= 0 && WorldLevel > LootPackageData.GetMaxWorldLevel())
            return false;

        if (LootPackageData.GetMinWorldLevel() >= 0 && WorldLevel < LootPackageData.GetMinWorldLevel())
            return false;
    }

    return true;
}

bool UFortLootPackage::PickLootDrops(TArray<FFortItemEntry>& OutLootToDrop, int32 WorldLevel, FName LootTierKey, const FGameplayTagContainer& GameplayTags, bool bAllowBonusLootDrops, bool bUseLootPreviewPackage)
{
    if (LootTierKey == NAME_None)
        return false;

    auto GameData = UFortGameData::Get();
    auto LootTierData = GameData->FindLootTierDataRow(LootTierKey, TEXT("UFortLootPackage::PickLootDrops"));

    if (!LootTierData || !GameData->HasLootPackageData())
        return false;

    const int32 NumInitialLootToDrop = OutLootToDrop.Num();
    const FName LootPackage = bUseLootPreviewPackage ? LootTierData->GetLootPreviewPackage() : LootTierData->GetLootPackage();

    UE_LOG(LogFortLoot, Verbose, TEXT("Selecting %f drops from %s"), LootTierData->GetNumLootPackageDrops(), *LootPackage.ToString());

    int32 NumLootPackageDrops = 1;

    if (LootPackage != NAME_None && LootTierData->GetNumLootPackageDrops() > 0.0f)
    {
        if (!bUseLootPreviewPackage && LootTierData->GetNumLootPackageDrops() >= 1.0f)
        {
            NumLootPackageDrops = FMath::FloorToInt(LootTierData->GetNumLootPackageDrops());

            const float Remainder = LootTierData->GetNumLootPackageDrops() - NumLootPackageDrops;

            if (Remainder > 1.e-5f)
            {
                NumLootPackageDrops += (Remainder >= FMath::FRand()) ? 1 : 0;
            }
        }

        UE_LOG(LogFortLoot, Verbose, TEXT("Actual number of loot drops is: %i"), NumLootPackageDrops);

        TMap<int32, int32> NumLootPackageDropsPerCategory;

        if (bUseLootPreviewPackage)
        {
            NumLootPackageDropsPerCategory.Add(0, 1);
        }
        else
        {
            UFortLootTier::GetNumLootPackageDropsPerCategory(LootTierData, NumLootPackageDrops, NumLootPackageDropsPerCategory);
        }

        for (auto& CategoryDrops : NumLootPackageDropsPerCategory)
        {
            UE_LOG(LogFortLoot, Verbose, TEXT("Dropping %i loot packages from drop category %i"), CategoryDrops.Value, CategoryDrops.Key);

            for (int32 DropIndex = 0; DropIndex < CategoryDrops.Value; ++DropIndex)
            {
                PickLootDropsFromLootPackage(OutLootToDrop, LootPackage, WorldLevel, CategoryDrops.Key, GameplayTags, !bUseLootPreviewPackage && bAllowBonusLootDrops && LootTierData->GetAllowBonusLootDrops());
            }
        }
    }

    const bool bAddedLootToDrop = OutLootToDrop.Num() > NumInitialLootToDrop;

    for (int32 Index = 0; Index < OutLootToDrop.Num(); ++Index)
    {
        if (OutLootToDrop[Index].GetItemDefinition() && !OutLootToDrop[Index].GetItemDefinition()->DoesAllowMultipleStacks())
        {
            for (int32 OtherIndex = Index + 1; OtherIndex < OutLootToDrop.Num(); ++OtherIndex)
            {
                if (OutLootToDrop[Index].GetItemDefinition() == OutLootToDrop[OtherIndex].GetItemDefinition())
                {
                    OutLootToDrop[Index].SetCount(OutLootToDrop[Index].GetCount() + OutLootToDrop[OtherIndex].GetCount());
                    OutLootToDrop.RemoveAt(OtherIndex);
                    --OtherIndex;
                }
            }
        }
    }

    return bAddedLootToDrop;
}

void UFortLootPackage::PickLootDropsFromLootPackage(TArray<FFortItemEntry>& OutLootToDrop, FName LootPackageName, int32 WorldLevel, int32 LootPackageCategory, const FGameplayTagContainer& GameplayTags, bool bAllowBonusDrops)
{
    TArray<TPair<FName, FFortLootPackageData*>> LootPackages;
    float TotalWeight = 0.0f;

    UFortGameData::Get()->FindLootPackageDataByPredicate(LootPackages, TotalWeight, [&](const FFortLootPackageData& LootPackageData)
    {
        return PackageMeetsCriteria(LootPackageData, LootPackageName, LootPackageCategory, GameplayTags, WorldLevel);
    });

    if (TotalWeight == 0.0f)
    {
        FString Tags;

        for (auto& GameplayTag : GameplayTags.GetGameplayTagArray())
        {
            Tags += GameplayTag.ToString();
            Tags += TEXT(" ");
        }

        UE_LOG(LogFortLoot, Warning, TEXT("Loot Package %s has no valid weights with the following data:\nLootPackageCategory: %d\nWorldLevel: %d\nTags: %s"), *LootPackageName.ToString(), LootPackageCategory, WorldLevel, *Tags);
    }

    float RandomWeight = FMath::FRand() * TotalWeight;

    for (auto& LootPackage : LootPackages)
    {
        auto LootPackageData = LootPackage.Value;

        if (RandomWeight <= LootPackageData->Weight)
        {
            UE_LOG(LogFortLoot, Verbose, TEXT("PickLootDropsFromLootPackage selected package %s"), *LootPackageData->LootPackageID.ToString());

            if (LootPackageData->GetLootPackageCall().IsEmpty())
            {
                if (!LootPackageData->GetItemDefinition().IsNull())
                {
                    if (auto ItemDefinition = LootPackageData->GetItemDefinition().LoadSynchronous())
                    {
                        int32 ItemLevel = 0;

                        if (auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition))
                        {
                            ItemLevel = UFortLootLevel::GetItemLevel(WorldItemDefinition->GetLootLevelData(), WorldLevel);
                        }

                        for (int32 NumberToDrop = LootPackageData->Count; NumberToDrop > 0;)
                        {
                            const int32 NumberToStack = FMath::Min(NumberToDrop, ItemDefinition->GetMaxStackSize());

                            auto ItemEntry = new(OutLootToDrop) FFortItemEntry(ItemDefinition, NumberToStack, FMath::Max(ItemLevel, 0));
                            ItemEntry->GetLevel() = ItemDefinition->ClampItemLevel(ItemEntry->GetLevel());

                            NumberToDrop -= NumberToStack;
                        }
                    }
                    else
                    {
                        UE_LOG(LogFortLoot, Warning, TEXT("Failed to load loot package %s."), *LootPackageData->LootPackageID.ToString());
                    }
                }
                else
                {
                    UE_LOG(LogFortLoot, Warning, TEXT("Loot Package %s does not contain a LootPackageCall or ItemDefinition."), *LootPackageData->LootPackageID.ToString());
                }
            }
            else
            {
                for (int32 Index = 0; Index < LootPackageData->Count; ++Index)
                {
                    PickLootDropsFromLootPackage(OutLootToDrop, FName(*LootPackageData->GetLootPackageCall()), WorldLevel, -1, GameplayTags, false);
                }
            }

            break;
        }

        RandomWeight -= LootPackageData->Weight;
    }
}
