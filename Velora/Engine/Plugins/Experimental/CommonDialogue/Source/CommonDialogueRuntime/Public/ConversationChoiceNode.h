#pragma once

#include "ConversationSubNode.h"
#include "ConversationContext.h"

class UConversationChoiceNode : public UConversationSubNode
{
public:
    static UClass* StaticClass();

    bool GenerateChoice(const FConversationContext& Context, FClientConversationOptionEntry& ChoiceEntry) const;

    void NotifyChoicePickedByUser(const FConversationContext& InContext, const FClientConversationOptionEntry& InClientChoice) const;
};
