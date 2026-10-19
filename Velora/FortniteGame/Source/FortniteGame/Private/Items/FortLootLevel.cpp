#include "pch.h"
#include "Items/FortLootLevel.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CoreMiscDefines.h"
#include "Scanner.h"

int32 UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel)
{
    static auto GetRows = Scanner::Function<void (*)(const FDataTableCategoryHandle&, TArray<FFortLootLevelData*>&, const FString&)>(Scanner::String(TEXT("UFortLootLevel::GetItemLevel")).Start().Find("lea r8, [0x*]").Call().Get(TEXT("FDataTableCategoryHandle::GetRows<FFortLootLevelData>")));

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
            if (LootLevelDataRows[Index]->LootLevel <= WorldLevel && LootLevelDataRows[Index]->LootLevel > BestLootLevel)
            {
                BestLootLevel = LootLevelDataRows[Index]->LootLevel;
                BestIndex = Index;
            }
        }

        if (BestIndex != INDEX_NONE)
            ItemLevel = FMath::RandRange(LootLevelDataRows[BestIndex]->MinItemLevel, LootLevelDataRows[BestIndex]->MaxItemLevel);
    }

    return ItemLevel;
}
