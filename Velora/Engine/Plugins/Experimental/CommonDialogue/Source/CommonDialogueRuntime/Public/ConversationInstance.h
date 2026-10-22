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

    UWorld* GetWorld() const;

    FConversationParticipants GetParticipantsCopy() const
    {
        return Participants;
    }

    const FConversationParticipantEntry* GetParticipant(const FGameplayTag& ParticipantTag) const
    {
        return Participants.GetParticipant(ParticipantTag);
    }

    const FConversationChoiceReference& GetCurrentChoiceReference() const
    {
        return CurrentBranchPoint.ClientChoice.ChoiceReference;
    }

    FConversationNodeHandle GetCurrentNodeHandle() const
    {
        return CurrentBranchPoint.GetNodeHandle();
    }

    const TArray<FClientConversationOptionEntry>& GetCurrentUserConversationChoices() const
    {
        return CurrentUserChoices;
    }

    void ServerRemoveParticipant(FGameplayTag ParticipantID);

    void ServerAssignParticipant(FGameplayTag ParticipantID, AActor* ParticipantActor);

    void ServerStartConversation(FGameplayTag EntryPoint);

    void ServerAdvanceConversation(const FAdvanceConversationRequest& InChoicePicked);

    void ServerAbortConversation();

    void ServerRefreshConversationChoices();

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

    FConversationParticipants Participants;
    FGameplayTag StartingEntryGameplayTag;
    FConversationBranchPoint StartingBranchPoint;
    FConversationBranchPoint CurrentBranchPoint;
    TArray<FCheckpoint> ClientBranchPoints;
    TArray<FConversationBranchPoint> CurrentBranchPoints;
    TArray<FClientConversationOptionEntry> CurrentUserChoices;
    TArray<FConversationChoiceReference> ScopeStack;
    FRandomStream ConversationRNG;
    bool bConversationStarted;
};
