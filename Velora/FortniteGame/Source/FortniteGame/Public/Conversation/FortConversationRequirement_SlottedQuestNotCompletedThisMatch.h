#pragma once

#include "ConversationRequirementNode.h"

class UFortConversationRequirement_SlottedQuestNotCompletedThisMatch : public UConversationRequirementNode
{
public:
    static UClass* StaticClass();

    EConversationRequirementResult IsRequirementSatisfied_Implementation(const FConversationContext& Context) const;
};
