#pragma once

#include "ConversationTaskNode.h"
#include "GameplayTagContainer.h"

class UConversationLinkNode : public UConversationTaskNode
{
public:
    static UClass* StaticClass();

    void GatherChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const;

    FGameplayTag GetRemoteEntryTag() const;
};
