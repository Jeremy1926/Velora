#include "pch.h"
#include "ConversationSideEffectNode.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UConversationSideEffectNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationSideEffectNode"));

    return Class;
}

void UConversationSideEffectNode::CauseSideEffect(const FConversationContext& Context) const
{
    if (Context.IsServerContext())
        ServerCauseSideEffect(Context);
}

void UConversationSideEffectNode::ServerCauseSideEffect(const FConversationContext& Context) const
{
    static auto ServerCauseSideEffect = Anchor::Function<void (*)(const UConversationSideEffectNode*, const FConversationContext&)>(TEXT("UConversationSideEffectNode::ServerCauseSideEffect"))
        .Event(TEXT("ServerCauseSideEffect"))
        .Get();

    ServerCauseSideEffect(this, Context);
}
