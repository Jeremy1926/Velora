#include "pch.h"
#include "Conversation/FortConversationTaskNode_GrantQuest.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_GrantQuest::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GrantQuest"));

    return Class;
}
