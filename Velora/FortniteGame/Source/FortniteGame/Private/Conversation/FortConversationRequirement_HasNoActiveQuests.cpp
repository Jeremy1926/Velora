#include "pch.h"
#include "Conversation/FortConversationRequirement_HasNoActiveQuests.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Player/FortPlayerController.h"
#include "Quests/FortQuestManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationRequirement_HasNoActiveQuests::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationRequirement_HasNoActiveQuests"));

    return Class;
}

EConversationRequirementResult UFortConversationRequirement_HasNoActiveQuests::IsRequirementSatisfied_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto QuestManager = PlayerController ? PlayerController->GetQuestManager(ESubGame::Athena) : nullptr;

    if (QuestManager && QuestManager->GetActiveTransientQuests().Num() == 0)
        return EConversationRequirementResult::Passed;

    return EConversationRequirementResult::FailedAndHidden;
}
