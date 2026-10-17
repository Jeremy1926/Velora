#pragma once

#include "HAL/Platform.h"

struct FDataTableCategoryHandle;

class UFortLootLevel
{
public:
    static int32 GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int32 WorldLevel);
};
