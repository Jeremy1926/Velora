#pragma once

#include "Items/FortItemDefinition.h"

class UFortWorldItem;
class AFortPlayerController;
struct FDataTableCategoryHandle;

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:
    static UClass* StaticClass();

    bool ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const;

    bool SupportsQuickbarFocus() const;

    bool CanBeDropped() const;

    bool ShouldDropOnDeath() const;

    float GetMaxDurability(int32 ItemLevel) const;

    const FDataTableCategoryHandle& GetLootLevelData() const;
};
