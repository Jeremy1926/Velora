#include "pch.h"
#include "Conversation/FortConversationTaskNode_Speech.h"
#include "ConversationParticipantComponent.h"
#include "FortConversationContextConditionHelpers.h"
#include "Kismet/KismetTextLibrary.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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
    Message.SpeakerID = SpeakerTag->GetPropertyValue_InContainer<FGameplayTag>(this);
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
    auto SpeakerTag = GetClass()->FindPropertyByName(FName(TEXT("SpeakerTag")));
    auto ConditionalMessageContainer = GetClass()->FindPropertyByName(FName(TEXT("ConditionalMessageContainer")));
    auto Message = GetClass()->FindPropertyByName(FName(TEXT("Message")));

    FText OutText;

    if (SpeakerTag && ResolveMessageForContext(SpeakerTag->GetPropertyValue_InContainer<FGameplayTag>(this), Context, OutText))
        return OutText;

    if (ConditionalMessageContainer && UFortConversationContextConditionHelpers::GetMessageForContext(ConditionalMessageContainer->ContainerPtrToValuePtr<void>(this), Context, OutText))
        return OutText;

    return Message ? *Message->ContainerPtrToValuePtr<FText>(this) : OutText;
}

bool UFortConversationTaskNode_Speech::ResolveMessageForContext(const FGameplayTag& SpeakerParticipantTag, const FConversationContext& ConversationContext, FText& OutText) const
{
    static auto ResolveMessageForContext = Anchor::Optional<bool (*)(const UFortConversationTaskNode_Speech*, const FGameplayTag&, const FConversationContext&, FText&)>(TEXT("UFortConversationTaskNode_Speech::ResolveMessageForContext"))
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode_Speech:ResolveMessageForContext"))
        .Find("setnz *; add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode_Speech:ResolveMessageForContext"))
        .Find("setnz *; add *, *; mov [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return ResolveMessageForContext && ResolveMessageForContext(this, SpeakerParticipantTag, ConversationContext, OutText);
}
