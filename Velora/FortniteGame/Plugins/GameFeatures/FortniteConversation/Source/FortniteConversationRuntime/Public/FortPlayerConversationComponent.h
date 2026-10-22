#pragma once

#include "ConversationParticipantComponent.h"

class UFortPlayerConversationComponent : public UConversationParticipantComponent
{
public:
    static UClass* StaticClass();

    void RequestServerAbortConversation_Implementation();
};
