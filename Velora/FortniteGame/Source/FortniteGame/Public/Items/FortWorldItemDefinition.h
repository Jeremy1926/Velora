#pragma once

#include "Items/FortItemDefinition.h"

class UFortWorldItem;
class AFortPlayerController;
struct FDataTableCategoryHandle;

enum class EWorldItemDropBehavior : uint8
{
    DropAsPickup,
    DestroyOnDrop,
    DropAsPickupDestroyOnEmpty,
    DropAsPickupEvenWhenEmpty,
    EWorldItemDropBehavior_MAX,
};

template<>
UEnum* StaticEnum<EWorldItemDropBehavior>();

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:
    static UClass* StaticClass();

    bool ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const;

    bool SupportsQuickbarFocus() const;

    bool CanBeDropped() const;

    bool ShouldDropOnDeath() const;

    EWorldItemDropBehavior GetDropBehavior() const;

    float GetMaxDurability(int32 ItemLevel) const;

    const FDataTableCategoryHandle& GetLootLevelData() const;
};
