#include "pch.h"
#include "Items/FortLootLevel.h"
#include "Engine/DataTable.h"
#include "Scanner.h"

int32 UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel)
{
    static auto GetItemLevel = Scanner::Function<int32 (*)(const FDataTableCategoryHandle&, int32)>(Scanner::String(TEXT("UFortLootLevel::GetItemLevel")).Start().Get(TEXT("UFortLootLevel::GetItemLevel")));

    return GetItemLevel(LootLevelData, WorldLevel);
}
