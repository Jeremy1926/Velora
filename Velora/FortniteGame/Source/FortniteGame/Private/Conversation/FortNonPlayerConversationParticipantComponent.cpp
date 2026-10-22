#include "pch.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortNonPlayerConversationParticipantComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortNonPlayerConversationParticipantComponent"));

    return Class;
}
