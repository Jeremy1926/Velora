#pragma once

#include "Conversation/FortConversationTaskNode.h"
#include "GameplayTagContainer.h"

class UFortControllerEffect;

class UFortConversationTaskNode_ApplyDataDrivenEffect : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    const FGameplayTag& GetParticipantID() const;

    const TArray<UFortControllerEffect*>& GetEffectsToApply() const;
};
