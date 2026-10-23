#include "pch.h"
#include "ConversationTypes.h"
#include "ConversationContext.h"
#include "ConversationParticipantComponent.h"
#include "ConversationRegistry.h"
#include "ConversationTaskNode.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UConversationNode* FConversationNodeHandle::TryToResolve(const FConversationContext& Context) const
{
    return Context.ConversationRegistry ? Context.ConversationRegistry->GetRuntimeNodeFromGUID(NodeGUID) : nullptr;
}

FClientConversationMessage::FClientConversationMessage(const FClientConversationMessage& Other)
{
    *this = Other;
}

FClientConversationMessage& FClientConversationMessage::operator=(const FClientConversationMessage& Other)
{
    static auto MetadataParametersProperty = StaticStruct()->FindPropertyByName(FName(TEXT("MetadataParameters")));

    SpeakerID = Other.SpeakerID;
    ParticipantDisplayName = Other.ParticipantDisplayName;
    Text = Other.Text;

    if (MetadataParametersProperty)
        MetadataParameters = Other.MetadataParameters;

    return *this;
}

UScriptStruct* FClientConversationMessage::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ClientConversationMessage"));

    return Struct;
}

bool FConversationChoiceReference::operator==(const FConversationChoiceReference& Other) const
{
    if (!(NodeReference == Other.NodeReference) || NodeParameters.Num() != Other.NodeParameters.Num())
        return false;

    for (int32 Index = 0; Index < NodeParameters.Num(); Index++)
    {
        if (!(NodeParameters[Index] == Other.NodeParameters[Index]))
            return false;
    }

    return true;
}

UConversationParticipantComponent* FConversationParticipantEntry::GetParticipantComponent() const
{
    return Actor ? (UConversationParticipantComponent*)Actor->FindComponentByClass(UConversationParticipantComponent::StaticClass()) : nullptr;
}

const FConversationParticipantEntry* FConversationParticipants::GetParticipant(const FGameplayTag& ParticipantID) const
{
    for (const FConversationParticipantEntry& Entry : List)
    {
        if (Entry.ParticipantID == ParticipantID)
            return &Entry;
    }

    return nullptr;
}

UConversationParticipantComponent* FConversationParticipants::GetParticipantComponent(const FGameplayTag& ParticipantID) const
{
    const FConversationParticipantEntry* Entry = GetParticipant(ParticipantID);

    return Entry ? Entry->GetParticipantComponent() : nullptr;
}

bool FConversationParticipants::Contains(AActor* PotentialParticipant) const
{
    for (const FConversationParticipantEntry& Entry : List)
    {
        if (Entry.Actor == PotentialParticipant)
            return true;
    }

    return false;
}

void FConversationBranchPointBuilder::AddChoice(const FConversationContext& InContext, const FClientConversationOptionEntry& InChoice)
{
    FConversationBranchPoint BranchPoint;
    BranchPoint.ClientChoice = InChoice;

    if (!BranchPoint.ClientChoice.ChoiceReference.NodeReference.IsValid() && InContext.TaskBeingConsidered)
        BranchPoint.ClientChoice.ChoiceReference.NodeReference = InContext.TaskBeingConsidered->GetNodeGuid();

    BranchPoint.ReturnScopeStack = InContext.ReturnScopeStack;

    BranchPoints.Add(BranchPoint);
}
