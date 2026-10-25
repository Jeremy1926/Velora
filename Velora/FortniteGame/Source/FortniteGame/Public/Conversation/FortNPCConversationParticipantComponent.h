#pragma once

#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "GameplayTagContainer.h"

class AFortAthenaAIBotController;
class AFortPlayerPawn;
class UConversationInstance;
class UFortItemDefinition;
class UFortConversationGiftTypeDefinitions;

class UFortNPCConversationParticipantComponent : public UFortNonPlayerConversationParticipantComponent
{
public:
    static UClass* StaticClass();

    void InitializeServerConversation(AFortAthenaAIBotController* BotController);

    void UpdateCanStartConversation();

    void OnServerConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant);

    void OnServerConversationEnded(UConversationInstance* Conversation);

    FGameplayTag GetNPCTag() const;

    TArray<const uint8*> GetSupportedGiftRows() const;

    TArray<const uint8*> GetSupportedGiftItemRows() const;

    UFortConversationGiftTypeDefinitions* GetGiftTypeDefinitions() const;

    AFortPlayerPawn* GetPlayerPawnOwner() const;

    AFortAthenaAIBotController*& GetBotControllerOwner() const;

    void SetConversationModeActive(bool bInConversationModeActive);
};
