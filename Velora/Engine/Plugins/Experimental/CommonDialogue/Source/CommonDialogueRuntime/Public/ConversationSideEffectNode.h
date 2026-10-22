#pragma once

#include "ConversationSubNode.h"
#include "ConversationContext.h"

class UConversationSideEffectNode : public UConversationSubNode
{
public:
    static UClass* StaticClass();

    void CauseSideEffect(const FConversationContext& Context) const;

    void ServerCauseSideEffect(const FConversationContext& Context) const;
};
