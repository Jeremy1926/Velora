#pragma once

#include "Conversation/FortConversationTaskNode_GrantStaticQuest.h"

class UFortConversationTaskNode_GrantPlayerBounty : public UFortConversationTaskNode_GrantStaticQuest
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    EConversationRequirementResult IsRequirementSatisfied_Implementation(const FConversationContext& Context) const;
};
