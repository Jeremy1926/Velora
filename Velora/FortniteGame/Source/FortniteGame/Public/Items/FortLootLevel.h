#pragma once

#include "HAL/Platform.h"
#include "Engine/DataTable.h"

struct FFortLootLevelData : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    int32 GetLootLevel() const;

    int32 GetMinItemLevel() const;

    int32 GetMaxItemLevel() const;
};

class UFortLootLevel
{
public:
    static int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel);
};
