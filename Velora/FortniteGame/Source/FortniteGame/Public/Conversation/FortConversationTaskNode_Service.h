#pragma once

#include "Conversation/FortConversationTaskNode.h"
#include "Misc/Optional.h"

class UCurveTable;
class UFortWorldItemDefinition;

class UFortConversationTaskNode_Service : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    UFortWorldItemDefinition* GetResourceCurrency() const;

    const TArray<UCurveTable*>& GetPricingTables() const;

    TOptional<int32> FindServicePrice(FName PricingRowKey, int32 PricingColumn) const;

    bool CanPlayerAffordPrice(const FConversationContext& Context, const TOptional<int32>& Price) const;

    bool PayPrice(const FConversationContext& Context, const TOptional<int32>& Price) const;
};
