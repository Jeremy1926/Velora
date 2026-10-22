#pragma once

#include "ConversationChoiceNode.h"

class UFortConversationChoiceWithPreviewNode : public UConversationChoiceNode
{
public:
    static UClass* StaticClass();

    bool GenerateChoice(const FConversationContext& Context, FClientConversationOptionEntry& ChoiceEntry) const;
};
