#pragma once

#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Misc/Guid.h"

class AActor;
class UConversationNode;
class UConversationParticipantComponent;
class UScriptStruct;
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
    FClientConversationMessage();

    FClientConversationMessage(const FClientConversationMessage& Other);

    ~FClientConversationMessage();

    FClientConversationMessage& operator=(const FClientConversationMessage& Other);

    static UScriptStruct* StaticStruct();

    FGameplayTag& GetSpeakerID() const;

    FText& GetParticipantDisplayName() const;

    FText& GetText() const;

private:
    alignas(void*) uint8 Data[0x48];
};

struct FClientConversationOptionEntry
{
    FClientConversationOptionEntry();

    FClientConversationOptionEntry(const FClientConversationOptionEntry& Other);

    ~FClientConversationOptionEntry();

    FClientConversationOptionEntry& operator=(const FClientConversationOptionEntry& Other);

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FText& GetChoiceText() const;

    EConversationChoiceType& GetChoiceType() const;

    FConversationChoiceReference& GetChoiceReference() const;

    TArray<FConversationNodeParameterPair>& GetExtraData() const;

    bool IsChoiceAvailable() const
    {
        return GetChoiceType() != EConversationChoiceType::UserChoiceUnavailable;
    }

    void SetChoiceAvailable(bool bIsAvailable)
    {
        GetChoiceType() = bIsAvailable ? EConversationChoiceType::UserChoiceAvailable : EConversationChoiceType::UserChoiceUnavailable;
    }

    bool operator==(const FConversationChoiceReference& Other) const
    {
        return GetChoiceReference() == Other;
    }

private:
    alignas(void*) uint8 Data[0x70];
};

template<>
class TArray<FClientConversationOptionEntry, FDefaultAllocator> : public TRuntimeSizedArray<FClientConversationOptionEntry>
{
public:
    TArray()
    {
    }

    TArray(const TArray& Other)
    {
        *this = Other;
    }
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
    FClientConversationMessagePayload();

    FClientConversationMessagePayload(const FClientConversationMessagePayload&) = delete;

    ~FClientConversationMessagePayload();

    static UScriptStruct* StaticStruct();

    FClientConversationMessage& GetMessage() const;

    FConversationParticipants& GetParticipants() const;

    FConversationNodeHandle& GetCurrentNode() const;

    TArray<FClientConversationOptionEntry>& GetOptions() const;

private:
    alignas(void*) uint8 Data[0x78];
};

struct FConversationBranchPoint
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FConversationNodeHandle GetNodeHandle() const
    {
        return ClientChoice.GetChoiceReference().NodeReference;
    }

    TArray<FConversationNodeHandle> ReturnScopeStack;
    FClientConversationOptionEntry ClientChoice;
};

template<>
class TArray<FConversationBranchPoint, FDefaultAllocator> : public TRuntimeSizedArray<FConversationBranchPoint>
{
public:
    TArray()
    {
    }

    TArray(const TArray& Other)
    {
        *this = Other;
    }
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
