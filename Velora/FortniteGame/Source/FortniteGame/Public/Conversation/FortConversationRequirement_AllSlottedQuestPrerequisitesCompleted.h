#pragma once

#include "ConversationRequirementNode.h"

class UFortConversationRequirement_AllSlottedQuestPrerequisitesCompleted : public UConversationRequirementNode
{
public:
    static UClass* StaticClass();

    EConversationRequirementResult IsRequirementSatisfied_Implementation(const FConversationContext& Context) const;
};
