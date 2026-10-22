#include "pch.h"
#include "Conversation/FortConversationTaskNode_Speech.h"
#include "ConversationParticipantComponent.h"
#include "FortConversationContextConditionHelpers.h"
#include "Kismet/KismetTextLibrary.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_Speech::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_Speech"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_Speech::ExecuteSpeechTaskNode(const FConversationContext& Context) const
{
    auto SpeakerTag = GetClass()->FindPropertyByName(FName(TEXT("SpeakerTag")));

    if (!SpeakerTag)
        return FConversationTaskResult::AbortConversation();

    FClientConversationMessage Message;
    Message.SpeakerID = *SpeakerTag->ContainerPtrToValuePtr<FGameplayTag>(this);
    Message.ParticipantDisplayName = GetParticipantNameStyled(Context, Message.SpeakerID);
    Message.Text = GetMessage(Context);

    return UConversationContextHelpers::PauseConversationAndSendClientChoices(Context, Message);
}

FText UFortConversationTaskNode_Speech::GetParticipantNameStyled(const FConversationContext& Context, FGameplayTag ParticipantTag) const
{
    auto ParticipantComponent = Context.GetParticipantComponent(ParticipantTag);
    const FText RawName = ParticipantComponent ? ParticipantComponent->GetParticipantDisplayName() : UKismetTextLibrary::Conv_StringToText(FString(TEXT("???")));

    return FText::Format(FTextFormat(UKismetTextLibrary::Conv_StringToText(FString(TEXT("<Name>{0}</>")))), FFormatArgumentValue(RawName));
}

FText UFortConversationTaskNode_Speech::GetMessage(const FConversationContext& Context) const
{
    auto ConditionalMessageContainer = GetClass()->FindPropertyByName(FName(TEXT("ConditionalMessageContainer")));
    auto Message = GetClass()->FindPropertyByName(FName(TEXT("Message")));

    FText OutText;

    if (ConditionalMessageContainer && UFortConversationContextConditionHelpers::GetMessageForContext(ConditionalMessageContainer->ContainerPtrToValuePtr<void>(this), Context, OutText))
        return OutText;

    return Message ? *Message->ContainerPtrToValuePtr<FText>(this) : OutText;
}
