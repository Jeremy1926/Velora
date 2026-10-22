#pragma once

#include "ConversationSubNode.h"
#include "ConversationContext.h"

class UConversationRequirementNode : public UConversationSubNode
{
public:
    static UClass* StaticClass();

    EConversationRequirementResult IsRequirementSatisfied(const FConversationContext& Context) const;
};
