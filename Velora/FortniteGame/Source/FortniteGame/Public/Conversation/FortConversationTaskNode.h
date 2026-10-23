#pragma once

#include "ConversationTaskNode.h"
#include "GameplayTagContainer.h"

class UFortConversationTaskNode : public UConversationTaskNode
{
public:
    static UClass* StaticClass();

    const FString& GetNodeType() const;

    const FGameplayTagContainer& GetNodeTags() const;

    bool HasPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option) const;

    void FillPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option, TArray<FConversationNodeParameterPair>& OutPreviewData) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
