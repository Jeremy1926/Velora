#include "pch.h"
#include "Conversation/FortConversationRequirement_SlottedQuestNotCompletedThisMatch.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "Player/FortPlayerController.h"
#include "Quests/FortQuestItemDefinition.h"
#include "Quests/FortQuestManager.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationRequirement_SlottedQuestNotCompletedThisMatch::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationRequirement_SlottedQuestNotCompletedThisMatch"));

    return Class;
}

EConversationRequirementResult UFortConversationRequirement_SlottedQuestNotCompletedThisMatch::IsRequirementSatisfied_Implementation(const FConversationContext& Context) const
{
    static auto QuestSlot = StaticClass()->FindPropertyByName(FName(TEXT("QuestSlot")));

    auto NPCComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto QuestManager = PlayerController ? PlayerController->GetQuestManager(ESubGame::Athena) : nullptr;
    const int32 Slot = *QuestSlot->ContainerPtrToValuePtr<int32>(this);

    if (!NPCComponent || !QuestManager || Slot < 1 || Slot > NPCComponent->GetSupportedQuests().Num())
        return EConversationRequirementResult::FailedAndHidden;

    auto SlottedQuest = NPCComponent->GetSupportedQuests()[Slot - 1].LoadSynchronous();

    if (!SlottedQuest || QuestManager->HasCompletedTransientQuest(SlottedQuest))
        return EConversationRequirementResult::FailedAndHidden;

    return EConversationRequirementResult::Passed;
}
