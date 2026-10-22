#include "pch.h"
#include "Conversation/FortConversationTaskNode_GrantQuest.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Athena/ControllerComponents/FortControllerComponent_TransientQuests.h"
#include "Player/FortPlayerController.h"
#include "Quests/FortQuestItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_GrantQuest::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GrantQuest"));

    return Class;
}

TSoftObjectPtr<UFortQuestItemDefinition> UFortConversationTaskNode_GrantQuest::GetQuestToGrant(const FConversationContext& Context) const
{
    static auto GetQuestToGrant = Anchor::VirtualFunction<TSoftObjectPtr<UFortQuestItemDefinition>* (*)(const UFortConversationTaskNode_GrantQuest*, TSoftObjectPtr<UFortQuestItemDefinition>*, const FConversationContext&)>(TEXT("UFortConversationTaskNode_GrantQuest::GetQuestToGrant"))
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_GrantQuest"), -0x18)
        .Find("call [*+0x*]")
        .Get();

    alignas(TSoftObjectPtr<UFortQuestItemDefinition>) uint8 QuestToGrantStorage[sizeof(TSoftObjectPtr<UFortQuestItemDefinition>)];
    auto QuestToGrant = GetQuestToGrant(this, (TSoftObjectPtr<UFortQuestItemDefinition>*)QuestToGrantStorage, Context);
    TSoftObjectPtr<UFortQuestItemDefinition> Result = *QuestToGrant;

    QuestToGrant->~TSoftObjectPtr();

    return Result;
}

FConversationTaskResult UFortConversationTaskNode_GrantQuest::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto TransientQuests = PlayerController ? PlayerController->FindComponentByClass<UFortControllerComponent_TransientQuests>() : nullptr;
    auto QuestToGrant = GetQuestToGrant(Context).LoadSynchronous();

    if (!TransientQuests || !QuestToGrant)
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    TransientQuests->GrantTransientQuest(QuestToGrant);

    return FConversationTaskResult::AdvanceConversation();
}
