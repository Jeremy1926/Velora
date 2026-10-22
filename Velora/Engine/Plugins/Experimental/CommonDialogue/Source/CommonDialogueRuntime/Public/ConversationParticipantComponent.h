#pragma once

#include "Components/ActorComponent.h"
#include "ConversationContext.h"

class UConversationInstance;

class UConversationParticipantComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerNotifyConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant);

    void ServerNotifyConversationEnded(UConversationInstance* Conversation);

    void ServerNotifyExecuteTaskAndSideEffects(const FConversationNodeHandle& Handle);

    void ServerForAllConversationsRefreshChoices(UConversationInstance* IgnoreConversation);

    void ServerAbortAllConversations();

    void SendClientConversationMessage(const FConversationContext& Context, const FClientConversationMessagePayload& Payload);

    void SendClientUpdatedChoices(const FConversationContext& Context);

    bool IsInActiveConversation() const;

    void ServerAdvanceConversation_Implementation(const FAdvanceConversationRequest& InChoicePicked);

    void ClientUpdateConversation(const FClientConversationMessagePayload& Message);

    void ClientUpdateConversations(int32 InConversationsActive);

    void ClientUpdateParticipants(const FConversationParticipants& InParticipants);

    void ClientExecuteTaskAndSideEffects(const FConversationNodeHandle& Handle);

    void OnEnterConversationState();

    void OnLeaveConversationState();

    void OnServerConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant);

    void OnServerConversationEnded(UConversationInstance* Conversation);

    int32& GetConversationsActive() const;

    UConversationInstance*& GetAuth_CurrentConversation() const;

    TArray<UConversationInstance*>& GetAuth_Conversations() const;
};
