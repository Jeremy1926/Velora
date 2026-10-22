#pragma once

#include "Items/FortAccountItem.h"

struct FVictoryCrownAccountData
{
    bool has_victory_crown;
    int32 total_victory_crowns_bestowed_count;
    int32 total_royal_royales_achieved_count;
    bool data_is_valid_for_mcp;
};

class UFortVictoryCrownAccountItem : public UFortAccountItem
{
public:
    static UClass* StaticClass();

    const FVictoryCrownAccountData& GetVictoryCrownAccountData() const;
};
