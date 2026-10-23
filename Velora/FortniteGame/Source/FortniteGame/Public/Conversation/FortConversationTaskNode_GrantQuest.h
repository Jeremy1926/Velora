#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "UObject/SoftObjectPtr.h"

class UFortQuestItemDefinition;

class UFortConversationTaskNode_GrantQuest : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    bool GetbShouldDisplayQuestProviderImage() const;

    TSoftObjectPtr<UFortQuestItemDefinition> GetQuestToGrant(const FConversationContext& Context) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
