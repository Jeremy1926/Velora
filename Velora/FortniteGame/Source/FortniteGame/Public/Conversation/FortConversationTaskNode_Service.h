#pragma once

#include "Conversation/FortConversationTaskNode.h"
#include "Misc/Optional.h"

class AFortPlayerController;
class UCurveTable;
class UFortWorldItemDefinition;

class UFortConversationTaskNode_Service : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    UFortWorldItemDefinition* GetResourceCurrency() const;

    const TArray<UCurveTable*>& GetPricingTables() const;

    TOptional<int32> FindServicePrice(FName PricingRowKey, int32 PricingColumn) const;

    int32 GetCurrencyCount(AFortPlayerController* PlayerController) const;

    bool CanPlayerAffordPrice(AFortPlayerController* PlayerController, int32 Price) const;

    bool PayPrice(AFortPlayerController* PlayerController, int32 Price) const;
};
