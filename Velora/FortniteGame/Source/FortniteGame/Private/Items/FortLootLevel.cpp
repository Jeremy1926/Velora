#include "pch.h"
#include "Items/FortLootLevel.h"
#include "Engine/DataTable.h"
#include "Scanner.h"

int32 UFortLootLevel::GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel)
{
    static auto GetItemLevel = (int32 (*)(const FDataTableCategoryHandle&, int32))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("UFortLootLevel::GetItemLevel")))->BeginAddress);

    return GetItemLevel(LootLevelData, WorldLevel);
}
