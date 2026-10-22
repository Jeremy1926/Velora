#pragma once

#include "ConversationTaskNode.h"

class UFortConversationTaskNode : public UConversationTaskNode
{
public:
    static UClass* StaticClass();

    const FString& GetNodeType() const;

    bool HasPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option) const;

    void FillPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option, TArray<FConversationNodeParameterPair>& OutPreviewData) const;
};
