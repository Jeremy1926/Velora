#include "pch.h"
#include "ConversationLinkNode.h"
#include "ConversationInstance.h"
#include "ConversationRegistry.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UConversationLinkNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationLinkNode"));

    return Class;
}

FGameplayTag UConversationLinkNode::GetRemoteEntryTag() const
{
    static auto RemoteEntryTag = StaticClass()->FindPropertyByName(FName(TEXT("RemoteEntryTag")));

    return RemoteEntryTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

void UConversationLinkNode::GatherChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const
{
    if (!Context.ConversationRegistry || !Context.ActiveConversation)
        return;

    TArray<FGuid> PotentialStartingPoints = Context.ConversationRegistry->GetOutputLinkGUIDs(GetRemoteEntryTag());

    if (PotentialStartingPoints.Num() > 0)
    {
        TArray<FGuid> LegalStartingPoints = Context.ActiveConversation->DetermineBranches(PotentialStartingPoints, EConversationRequirementResult::FailedButVisible);
        FConversationContext ReturnScopeContext = Context.CreateReturnScopeContext(GetNodeGuid());

        UConversationTaskNode::GenerateChoicesForDestinations(BranchBuilder, ReturnScopeContext, LegalStartingPoints);
    }
}
