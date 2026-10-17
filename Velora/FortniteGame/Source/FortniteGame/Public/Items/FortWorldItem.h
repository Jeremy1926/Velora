#pragma once

#include "Items/FortItem.h"

struct FFortItemEntry;

class UFortWorldItem : public UFortItem
{
public:
    void SetLoadedAmmo(int32 NewLoadedAmmo);

    FFortItemEntry& GetItemEntry();
};
