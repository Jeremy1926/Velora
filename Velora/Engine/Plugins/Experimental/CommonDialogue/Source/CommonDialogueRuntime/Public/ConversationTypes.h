#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/Guid.h"

class AActor;
class UConversationNode;
class UConversationParticipantComponent;
struct FConversationContext;

enum class EConversationTaskResultType : uint8
{
    Invalid,
    AbortConversation,
    AdvanceConversation,
    AdvanceConversationWithChoice,
    PauseConversationAndSendClientChoices,
    ReturnToLastClientChoice,
    ReturnToCurrentClientChoice,
    ReturnToConversationStart
};

enum class EConversationRequirementResult : uint8
{
    Passed,
    FailedButVisible,
    FailedAndHidden
};

enum class EConversationChoiceType : uint8
{
    ServerOnly,
    UserChoiceAvailable,
    UserChoiceUnavailable
};

struct FConversationNodeHandle
{
    FConversationNodeHandle() = default;

    FConversationNodeHandle(const FGuid& InNodeGUID)
        : NodeGUID(InNodeGUID)
    {
    }

    bool IsValid() const
    {
        return NodeGUID.IsValid();
    }

    bool operator==(const FConversationNodeHandle& Other) const
    {
        return NodeGUID == Other.NodeGUID;
    }

    UConversationNode* TryToResolve(const FConversationContext& Context) const;

    FGuid NodeGUID;
};

struct FConversationNodeParameterPair
{
    bool operator==(const FConversationNodeParameterPair& Other) const
    {
        return Name == Other.Name && Value == Other.Value;
    }

    FString Name;
    FString Value;
};

struct FConversationChoiceReference
{
    FConversationChoiceReference() = default;

    FConversationChoiceReference(const FConversationNodeHandle& InNodeReference)
        : NodeReference(InNodeReference)
    {
    }

    bool IsValid() const
    {
        return NodeReference.IsValid();
    }

    bool operator==(const FConversationChoiceReference& Other) const;

    FConversationNodeHandle NodeReference;
    TArray<FConversationNodeParameterPair> NodeParameters;
};

struct FAdvanceConversationRequest
{
    FConversationChoiceReference Choice;
    TArray<FConversationNodeParameterPair> UserParameters;
};

struct FClientConversationMessage
{
    FGameplayTag SpeakerID;
    FText ParticipantDisplayName;
    FText Text;
};

struct FClientConversationOptionEntry
{
    bool IsChoiceAvailable() const
    {
        return ChoiceType != EConversationChoiceType::UserChoiceUnavailable;
    }

    void SetChoiceAvailable(bool bIsAvailable)
    {
        ChoiceType = bIsAvailable ? EConversationChoiceType::UserChoiceAvailable : EConversationChoiceType::UserChoiceUnavailable;
    }

    bool operator==(const FConversationChoiceReference& Other) const
    {
        return ChoiceReference == Other;
    }

    FText ChoiceText;
    FGameplayTagContainer ChoiceTags;
    EConversationChoiceType ChoiceType = EConversationChoiceType::ServerOnly;
    FConversationChoiceReference ChoiceReference;
    TArray<FConversationNodeParameterPair> ExtraData;
};

struct FConversationParticipantEntry
{
    UConversationParticipantComponent* GetParticipantComponent() const;

    AActor* Actor = nullptr;
    FGameplayTag ParticipantID;
};

struct FConversationParticipants
{
    const FConversationParticipantEntry* GetParticipant(const FGameplayTag& ParticipantID) const;

    UConversationParticipantComponent* GetParticipantComponent(const FGameplayTag& ParticipantID) const;

    bool Contains(AActor* PotentialParticipant) const;

    TArray<FConversationParticipantEntry> List;
};

struct FClientConversationMessagePayload
{
    FClientConversationMessage Message;
    FConversationParticipants Participants;
    FConversationNodeHandle CurrentNode;
    TArray<FClientConversationOptionEntry> Options;
};

struct FConversationBranchPoint
{
    FConversationNodeHandle GetNodeHandle() const
    {
        return ClientChoice.ChoiceReference.NodeReference;
    }

    TArray<FConversationNodeHandle> ReturnScopeStack;
    FClientConversationOptionEntry ClientChoice;
};

struct FConversationBranchPointBuilder
{
    void AddChoice(const FConversationContext& InContext, const FClientConversationOptionEntry& InChoice);

    int32 Num() const
    {
        return BranchPoints.Num();
    }

    TArray<FConversationBranchPoint> BranchPoints;
};
