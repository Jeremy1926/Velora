#include "pch.h"
#include "ConversationChoiceNode.h"
#include "Conversation/FortConversationChoiceWithPreviewNode.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UConversationChoiceNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationChoiceNode"));

    return Class;
}

bool UConversationChoiceNode::GenerateChoice(const FConversationContext& Context, FClientConversationOptionEntry& ChoiceEntry) const
{
    static auto GenerateChoice = Anchor::VirtualFunction<bool (*)(const UConversationChoiceNode*, const FConversationContext&, FClientConversationOptionEntry&)>(TEXT("UConversationChoiceNode::GenerateChoice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationChoiceNode:FillChoice"))
        .Find("call [*+0x*]")
        .Get(sizeof(void*));

    if (auto ChoiceWithPreviewNode = Cast<UFortConversationChoiceWithPreviewNode>(this))
        return ChoiceWithPreviewNode->GenerateChoice(Context, ChoiceEntry);

    return GenerateChoice(this, Context, ChoiceEntry);
}
