#pragma once

#include "UObject/Object.h"
#include "ConversationContext.h"
#include "Math/RandomStream.h"

class AActor;

class UConversationInstance : public UObject
{
public:
    struct FCheckpoint
    {
        FConversationBranchPoint ClientBranchPoint;
        TArray<FConversationChoiceReference> ScopeStack;
    };

    static UClass* StaticClass();

    static bool HasServerLayout();

    FConversationParticipants GetParticipantsCopy() const
    {
        return GetParticipants();
    }

    const FConversationParticipantEntry* GetParticipant(const FGameplayTag& ParticipantTag) const
    {
        return GetParticipants().GetParticipant(ParticipantTag);
    }

    const FConversationChoiceReference& GetCurrentChoiceReference() const
    {
        return GetCurrentBranchPoint().ClientChoice.ChoiceReference;
    }

    FConversationNodeHandle GetCurrentNodeHandle() const
    {
        return GetCurrentBranchPoint().GetNodeHandle();
    }

    const TArray<FClientConversationOptionEntry>& GetCurrentUserConversationChoices() const
    {
        return GetCurrentUserChoices();
    }

    void ServerRemoveParticipant(FGameplayTag ParticipantID);

    void ServerAssignParticipant(FGameplayTag ParticipantID, AActor* ParticipantActor);

    void ServerStartConversation(FGameplayTag EntryPoint);

    void ServerAdvanceConversation(const FAdvanceConversationRequest& InChoicePicked);

    void ServerAbortConversation();

    void PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& ClientMessage);

    void ReturnToLastClientChoice(const FConversationContext& Context);

    void ReturnToCurrentClientChoice(const FConversationContext& Context);

    void ReturnToStart(const FConversationContext& Context);

    TArray<FGuid> DetermineBranches(const TArray<FGuid>& SourceList, EConversationRequirementResult MaximumRequirementResult);

private:
    void TryStartingConversation();

    void ResetConversationProgress();

    void UpdateNextChoices(const FConversationContext& Context);

    void SetNextChoices(const TArray<FConversationBranchPoint>& InAllChoices);

    const FConversationBranchPoint* FindBranchPointFromClientChoice(const FConversationChoiceReference& InChoice) const;

    void ModifyCurrentConversationNode(const FConversationChoiceReference& NewChoice);

    void ModifyCurrentConversationNode(const FConversationBranchPoint& NewBranchPoint);

    void OnCurrentConversationNodeModified();

    uint8* GetLayoutMember(int32 OldLayoutOffset, int32 NewLayoutOffset) const;

    FConversationParticipants& GetParticipants() const;

    FGameplayTag& GetStartingEntryGameplayTag() const;

    FConversationBranchPoint& GetStartingBranchPoint() const;

    FConversationBranchPoint& GetCurrentBranchPoint() const;

    TArray<FCheckpoint>& GetClientBranchPoints() const;

    TArray<FConversationBranchPoint>& GetCurrentBranchPoints() const;

    TArray<FClientConversationOptionEntry>& GetCurrentUserChoices() const;

    TArray<FConversationChoiceReference>& GetScopeStack() const;

    FRandomStream& GetConversationRNG() const;

    bool& GetbConversationStarted() const;
};
