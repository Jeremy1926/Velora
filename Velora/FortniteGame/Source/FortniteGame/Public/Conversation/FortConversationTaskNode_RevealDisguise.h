#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class UFortConversationTaskNode_RevealDisguise : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;
};
