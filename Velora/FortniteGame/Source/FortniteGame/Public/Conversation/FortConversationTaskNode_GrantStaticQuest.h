#pragma once

#include "Conversation/FortConversationTaskNode_GrantQuest.h"
#include "UObject/SoftObjectPtr.h"

class UFortQuestItemDefinition;

class UFortConversationTaskNode_GrantStaticQuest : public UFortConversationTaskNode_GrantQuest
{
public:
    static UClass* StaticClass();

    const TSoftObjectPtr<UFortQuestItemDefinition>& GetQuestToGrantPtr() const;
};
