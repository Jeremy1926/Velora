#pragma once

#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

class AFortAthenaAIBotController;
class AFortPlayerPawn;
class UConversationInstance;
class UDataTable;
class UFortItemDefinition;
class UFortQuestItemDefinition;
class UFortConversationGiftTypeDefinitions;

class UFortNPCConversationParticipantComponent : public UFortNonPlayerConversationParticipantComponent
{
public:
    static UClass* StaticClass();

    void InitializeServerConversation(AFortAthenaAIBotController* BotController);

    void InitializeServices();

    void UpdateCanStartConversation();

    void OnServerConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant);

    void OnServerConversationEnded(UConversationInstance* Conversation);

    FGameplayTag GetNPCTag() const;

    TArray<const uint8*> GetSupportedSaleRows() const;

    TArray<const uint8*> GetSupportedGiftRows() const;

    TArray<const uint8*> GetSupportedGiftItemRows() const;

    UFortConversationGiftTypeDefinitions* GetGiftTypeDefinitions() const;

    AFortPlayerPawn* GetPlayerPawnOwner() const;

    AFortAthenaAIBotController*& GetBotControllerOwner() const;

    TArray<TSoftObjectPtr<UFortQuestItemDefinition>>& GetSupportedQuests() const;

    FGameplayTagContainer& GetSupportedServices() const;

    UDataTable*& GetServices() const;

    UDataTable*& GetSalesInventory() const;

    UDataTable** GetQuests() const;

    void SetConversationModeActive(bool bInConversationModeActive);
};
