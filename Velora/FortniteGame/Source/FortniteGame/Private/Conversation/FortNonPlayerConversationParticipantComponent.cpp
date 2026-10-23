#include "pch.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortNonPlayerConversationParticipantComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortNonPlayerConversationParticipantComponent"));

    return Class;
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetConversationEntryTag() const
{
    static auto ConversationEntryTag = StaticClass()->FindPropertyByName(FName(TEXT("ConversationEntryTag")));

    return ConversationEntryTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetInteractorParticipantTag() const
{
    static auto InteractorParticipantTag = StaticClass()->FindPropertyByName(FName(TEXT("InteractorParticipantTag")));

    return InteractorParticipantTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetSelfParticipantTag() const
{
    static auto SelfParticipantTag = StaticClass()->FindPropertyByName(FName(TEXT("SelfParticipantTag")));

    return SelfParticipantTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}
