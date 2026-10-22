#include "pch.h"
#include "Conversation/FortConversationTaskNode_GrantStaticQuest.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_GrantStaticQuest::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GrantStaticQuest"));

    return Class;
}

const TSoftObjectPtr<UFortQuestItemDefinition>& UFortConversationTaskNode_GrantStaticQuest::GetQuestToGrantPtr() const
{
    static auto QuestToGrantPtr = StaticClass()->FindPropertyByName(FName(TEXT("QuestToGrantPtr")));

    return *QuestToGrantPtr->ContainerPtrToValuePtr<TSoftObjectPtr<UFortQuestItemDefinition>>(this);
}
