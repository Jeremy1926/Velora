#include "pch.h"
#include "Items/FortLootLevel.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FFortLootLevelData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortLootLevelData"));

    return Struct;
}

int32 FFortLootLevelData::GetLootLevel() const
{
    static auto LootLevel = StaticStruct()->FindPropertyByName(FName(TEXT("LootLevel")));

    return *LootLevel->ContainerPtrToValuePtr<int32>(this);
}

int32 FFortLootLevelData::GetMinItemLevel() const
{
    static auto MinItemLevel = StaticStruct()->FindPropertyByName(FName(TEXT("MinItemLevel")));

    return *MinItemLevel->ContainerPtrToValuePtr<int32>(this);
}

int32 FFortLootLevelData::GetMaxItemLevel() const
{
    static auto MaxItemLevel = StaticStruct()->FindPropertyByName(FName(TEXT("MaxItemLevel")));

    return *MaxItemLevel->ContainerPtrToValuePtr<int32>(this);
}

int32 UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel)
{
    static auto GetItemLevel = Anchor::Optional<int32 (*)(const FDataTableCategoryHandle&, int32)>(TEXT("UFortLootLevel::GetItemLevel"))
        .String(TEXT("UFortLootLevel::GetItemLevel"))
        .Start()
        .Find("lea r8, [0x*]")
        .Last("cmp *[rcx], *")
        .Start()
        .Get();

    if (GetItemLevel)
        return GetItemLevel(LootLevelData, WorldLevel);

    static auto GetRows = Anchor::Function<void (*)(const FDataTableCategoryHandle&, TArray<FFortLootLevelData*>&, const FString&)>(TEXT("FDataTableCategoryHandle::GetRows<FFortLootLevelData>"))
        .String(TEXT("UFortLootLevel::GetItemLevel"))
        .Start()
        .Find("lea r8, [0x*]")
        .Call()
        .Get();

    int32 ItemLevel = 0;

    if (LootLevelData.DataTable)
    {
        static const FString ContextString(TEXT("UFortLootLevel::GetItemLevel"));

        TArray<FFortLootLevelData*> LootLevelDataRows;
        GetRows(LootLevelData, LootLevelDataRows, ContextString);

        int32 BestLootLevel = INDEX_NONE;
        int32 BestIndex = INDEX_NONE;

        for (int32 Index = 0; Index < LootLevelDataRows.Num(); Index++)
        {
            if (LootLevelDataRows[Index]->GetLootLevel() <= WorldLevel && LootLevelDataRows[Index]->GetLootLevel() > BestLootLevel)
            {
                BestLootLevel = LootLevelDataRows[Index]->GetLootLevel();
                BestIndex = Index;
            }
        }

        if (BestIndex != INDEX_NONE)
            ItemLevel = FMath::RandRange(LootLevelDataRows[BestIndex]->GetMinItemLevel(), LootLevelDataRows[BestIndex]->GetMaxItemLevel());
    }

    return ItemLevel;
}
