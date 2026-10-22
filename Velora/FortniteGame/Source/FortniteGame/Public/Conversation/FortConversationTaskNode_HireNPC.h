#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class UFortConversationTaskNode_HireNPC : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;
};
