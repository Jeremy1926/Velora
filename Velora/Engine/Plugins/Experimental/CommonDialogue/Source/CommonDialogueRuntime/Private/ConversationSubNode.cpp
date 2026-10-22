#include "pch.h"
#include "ConversationSubNode.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UConversationSubNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationSubNode"));

    return Class;
}
