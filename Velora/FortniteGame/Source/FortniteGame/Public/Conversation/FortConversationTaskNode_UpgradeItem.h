#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

struct FWeaponUpgradeItemRow;

class UFortConversationTaskNode_UpgradeItem : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    const FWeaponUpgradeItemRow* GetUpgradedItemRow(const FConversationContext& Context) const;

    TOptional<int32> GetUpgradePrice(const FWeaponUpgradeItemRow* UpgradedItemRow) const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;
};
