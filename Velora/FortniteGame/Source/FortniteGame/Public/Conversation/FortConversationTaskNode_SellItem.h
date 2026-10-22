#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "UObject/WeakObjectPtr.h"

class UFortItemDefinition;

struct FConversationSellTaskMemory
{
    TWeakObjectPtr<UFortItemDefinition> LootForSaleDefinition;
    int32 LootForSaleStackSize = 0;
    int32 OriginalLootCountForSale = 0;
    int32 LootRemainingForSale = 0;
};

class UFortConversationTaskNode_SellItem : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    int32 GetSellSlot() const;

    const FText& GetBuyTextFormat() const;

    const FText& GetSoldOutTextFormat() const;

    FConversationSellTaskMemory* GetSellingData(const FConversationContext& Context) const;

    TOptional<int32> GetLootCost(const FConversationSellTaskMemory& SellingData) const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    EConversationRequirementResult IsRequirementSatisfied_Implementation(const FConversationContext& Context) const;

    void GatherDynamicChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const;
};
