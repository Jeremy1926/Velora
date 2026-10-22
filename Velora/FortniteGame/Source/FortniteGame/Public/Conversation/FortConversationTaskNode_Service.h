#pragma once

#include "Conversation/FortConversationTaskNode.h"

class UCurveTable;
class UFortWorldItemDefinition;

class UFortConversationTaskNode_Service : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    UFortWorldItemDefinition* GetResourceCurrency() const;

    const TArray<UCurveTable*>& GetPricingTables() const;
};
