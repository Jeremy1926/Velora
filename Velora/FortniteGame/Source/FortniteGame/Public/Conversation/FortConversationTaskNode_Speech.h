#pragma once

#include "Conversation/FortConversationTaskNode.h"

class UFortConversationTaskNode_Speech : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteSpeechTaskNode(const FConversationContext& Context) const;

    FText GetParticipantNameStyled(const FConversationContext& Context, FGameplayTag ParticipantTag) const;

    FText GetMessage(const FConversationContext& Context) const;

    bool ResolveMessageForContext(const FGameplayTag& SpeakerParticipantTag, const FConversationContext& ConversationContext, FText& OutText) const;
};
