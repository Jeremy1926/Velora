#include "pch.h"
#include "ConversationRequirementNode.h"
#include "Conversation/FortConversationRequirement_AllSlottedQuestPrerequisitesCompleted.h"
#include "Conversation/FortConversationRequirement_HasNoActiveQuests.h"
#include "Conversation/FortConversationRequirement_HasService.h"
#include "Conversation/FortConversationRequirement_SlottedQuestNotCompletedThisMatch.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UConversationRequirementNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationRequirementNode"));

    return Class;
}

EConversationRequirementResult UConversationRequirementNode::IsRequirementSatisfied(const FConversationContext& Context) const
{
    static auto IsRequirementSatisfied_Implementation = Anchor::VirtualFunction<EConversationRequirementResult (*)(const UConversationRequirementNode*, const FConversationContext&)>(TEXT("UConversationRequirementNode::IsRequirementSatisfied_Implementation"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationRequirementNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .Get();

    if (auto HasService = Cast<UFortConversationRequirement_HasService>(this))
        return HasService->IsRequirementSatisfied_Implementation(Context);

    if (auto HasNoActiveQuests = Cast<UFortConversationRequirement_HasNoActiveQuests>(this))
        return HasNoActiveQuests->IsRequirementSatisfied_Implementation(Context);

    if (auto SlottedQuestNotCompletedThisMatch = Cast<UFortConversationRequirement_SlottedQuestNotCompletedThisMatch>(this))
        return SlottedQuestNotCompletedThisMatch->IsRequirementSatisfied_Implementation(Context);

    if (auto AllSlottedQuestPrerequisitesCompleted = Cast<UFortConversationRequirement_AllSlottedQuestPrerequisitesCompleted>(this))
        return AllSlottedQuestPrerequisitesCompleted->IsRequirementSatisfied_Implementation(Context);

    return IsRequirementSatisfied_Implementation(this, Context);
}
