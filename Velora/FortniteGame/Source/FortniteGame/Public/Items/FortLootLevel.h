#pragma once

#include "HAL/Platform.h"
#include "Engine/DataTable.h"

struct FFortLootLevelData : public FTableRowBase
{
    FName Category;
    int32 LootLevel;
    int32 MinItemLevel;
    int32 MaxItemLevel;
};

class UFortLootLevel
{
public:
    static int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel);
};
