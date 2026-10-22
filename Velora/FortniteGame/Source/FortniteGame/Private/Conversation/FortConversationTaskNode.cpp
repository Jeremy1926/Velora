#include "pch.h"
#include "Conversation/FortConversationTaskNode.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode"));

    return Class;
}

const FString& UFortConversationTaskNode::GetNodeType() const
{
    static auto NodeType = StaticClass()->FindPropertyByName(FName(TEXT("NodeType")));

    return *NodeType->ContainerPtrToValuePtr<FString>(this);
}

bool UFortConversationTaskNode::HasPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option) const
{
    static auto HasPreviewData = Anchor::VirtualFunction<bool (*)(const UFortConversationTaskNode*, const FConversationContext&, const FClientConversationOptionEntry&)>(TEXT("UFortConversationTaskNode::HasPreviewData"))
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .Get(-0x10);

    return HasPreviewData(this, Context, Option);
}

void UFortConversationTaskNode::FillPreviewData(const FConversationContext& Context, const FClientConversationOptionEntry& Option, TArray<FConversationNodeParameterPair>& OutPreviewData) const
{
    static auto FillPreviewData = Anchor::VirtualFunction<void (*)(const UFortConversationTaskNode*, const FConversationContext&, const FClientConversationOptionEntry&, TArray<FConversationNodeParameterPair>&)>(TEXT("UFortConversationTaskNode::FillPreviewData"))
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .Get(-0x8);

    FillPreviewData(this, Context, Option, OutPreviewData);
}
