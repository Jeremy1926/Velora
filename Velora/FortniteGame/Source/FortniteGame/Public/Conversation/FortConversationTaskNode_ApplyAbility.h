#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class UFortConversationTaskNode_ApplyAbility : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    UClass* GetGrantedAbility() const;

    int32 GetLevel() const;

    bool GetbActivateImmediatelyOnGranting() const;

    FName GetPricingKey() const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;
};
