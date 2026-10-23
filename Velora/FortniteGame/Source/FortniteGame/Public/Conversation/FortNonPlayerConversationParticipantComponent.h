#pragma once

#include "ConversationParticipantComponent.h"
#include "GameplayTagContainer.h"

class UFortNonPlayerConversationParticipantComponent : public UConversationParticipantComponent
{
public:
    static UClass* StaticClass();

    FGameplayTag GetConversationEntryTag() const;

    FGameplayTag GetInteractorParticipantTag() const;

    FGameplayTag GetSelfParticipantTag() const;
};
