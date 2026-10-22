#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "UObject/SoftObjectPtr.h"

class UFortQuestItemDefinition;

class UFortConversationTaskNode_GrantQuest : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    TSoftObjectPtr<UFortQuestItemDefinition> GetQuestToGrant(const FConversationContext& Context) const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;
};
