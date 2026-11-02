#include "pch.h"
#include "ConversationTypes.h"
#include "ConversationContext.h"
#include "ConversationParticipantComponent.h"
#include "ConversationRegistry.h"
#include "ConversationTaskNode.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UConversationNode* FConversationNodeHandle::TryToResolve(const FConversationContext& Context) const
{
    return Context.ConversationRegistry ? Context.ConversationRegistry->GetRuntimeNodeFromGUID(NodeGUID) : nullptr;
}

FClientConversationMessage::FClientConversationMessage()
{
    StaticStruct()->InitializeStruct(this);
}

FClientConversationMessage::FClientConversationMessage(const FClientConversationMessage& Other)
    : FClientConversationMessage()
{
    *this = Other;
}

FClientConversationMessage::~FClientConversationMessage()
{
    StaticStruct()->DestroyStruct(this);
}

FClientConversationMessage& FClientConversationMessage::operator=(const FClientConversationMessage& Other)
{
    StaticStruct()->CopyScriptStruct(this, &Other);

    return *this;
}

UScriptStruct* FClientConversationMessage::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ClientConversationMessage"));

    return Struct;
}

FGameplayTag& FClientConversationMessage::GetSpeakerID() const
{
    static auto SpeakerID = StaticStruct()->FindPropertyByName(FName(TEXT("SpeakerID")));

    return *SpeakerID->ContainerPtrToValuePtr<FGameplayTag>((void*)this);
}

FText& FClientConversationMessage::GetParticipantDisplayName() const
{
    static auto ParticipantDisplayName = StaticStruct()->FindPropertyByName(FName(TEXT("ParticipantDisplayName")));

    return *ParticipantDisplayName->ContainerPtrToValuePtr<FText>((void*)this);
}

FText& FClientConversationMessage::GetText() const
{
    static auto Text = StaticStruct()->FindPropertyByName(FName(TEXT("Text")));

    return *Text->ContainerPtrToValuePtr<FText>((void*)this);
}

FClientConversationOptionEntry::FClientConversationOptionEntry()
{
    StaticStruct()->InitializeStruct(this);
}

FClientConversationOptionEntry::FClientConversationOptionEntry(const FClientConversationOptionEntry& Other)
    : FClientConversationOptionEntry()
{
    *this = Other;
}

FClientConversationOptionEntry::~FClientConversationOptionEntry()
{
    StaticStruct()->DestroyStruct(this);
}

FClientConversationOptionEntry& FClientConversationOptionEntry::operator=(const FClientConversationOptionEntry& Other)
{
    StaticStruct()->CopyScriptStruct(this, &Other);

    return *this;
}

UScriptStruct* FClientConversationOptionEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ClientConversationOptionEntry"));

    return Struct;
}

int32 FClientConversationOptionEntry::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FText& FClientConversationOptionEntry::GetChoiceText() const
{
    static auto ChoiceText = StaticStruct()->FindPropertyByName(FName(TEXT("ChoiceText")));

    return *ChoiceText->ContainerPtrToValuePtr<FText>((void*)this);
}

EConversationChoiceType& FClientConversationOptionEntry::GetChoiceType() const
{
    static auto ChoiceType = StaticStruct()->FindPropertyByName(FName(TEXT("ChoiceType")));

    return *ChoiceType->ContainerPtrToValuePtr<EConversationChoiceType>((void*)this);
}

FConversationChoiceReference& FClientConversationOptionEntry::GetChoiceReference() const
{
    static auto ChoiceReference = StaticStruct()->FindPropertyByName(FName(TEXT("ChoiceReference")));

    return *ChoiceReference->ContainerPtrToValuePtr<FConversationChoiceReference>((void*)this);
}

TArray<FConversationNodeParameterPair>& FClientConversationOptionEntry::GetExtraData() const
{
    static auto ExtraData = StaticStruct()->FindPropertyByName(FName(TEXT("ExtraData")));

    return *ExtraData->ContainerPtrToValuePtr<TArray<FConversationNodeParameterPair>>((void*)this);
}

FClientConversationMessagePayload::FClientConversationMessagePayload()
{
    StaticStruct()->InitializeStruct(this);
}

FClientConversationMessagePayload::~FClientConversationMessagePayload()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FClientConversationMessagePayload::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ClientConversationMessagePayload"));

    return Struct;
}

FClientConversationMessage& FClientConversationMessagePayload::GetMessage() const
{
    static auto Message = StaticStruct()->FindPropertyByName(FName(TEXT("Message")));

    return *Message->ContainerPtrToValuePtr<FClientConversationMessage>((void*)this);
}

FConversationParticipants& FClientConversationMessagePayload::GetParticipants() const
{
    static auto Participants = StaticStruct()->FindPropertyByName(FName(TEXT("Participants")));

    return *Participants->ContainerPtrToValuePtr<FConversationParticipants>((void*)this);
}

FConversationNodeHandle& FClientConversationMessagePayload::GetCurrentNode() const
{
    static auto CurrentNode = StaticStruct()->FindPropertyByName(FName(TEXT("CurrentNode")));

    return *CurrentNode->ContainerPtrToValuePtr<FConversationNodeHandle>((void*)this);
}

TArray<FClientConversationOptionEntry>& FClientConversationMessagePayload::GetOptions() const
{
    static auto Options = StaticStruct()->FindPropertyByName(FName(TEXT("Options")));

    return *Options->ContainerPtrToValuePtr<TArray<FClientConversationOptionEntry>>((void*)this);
}

UScriptStruct* FConversationBranchPoint::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationBranchPoint"));

    return Struct;
}

int32 FConversationBranchPoint::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
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

    if (!BranchPoint.ClientChoice.GetChoiceReference().NodeReference.IsValid() && InContext.TaskBeingConsidered)
        BranchPoint.ClientChoice.GetChoiceReference().NodeReference = InContext.TaskBeingConsidered->GetNodeGuid();

    BranchPoint.ReturnScopeStack = InContext.ReturnScopeStack;

    BranchPoints.Add(BranchPoint);
}
