#pragma once

#include "ConversationParticipantComponent.h"
#include "GameplayTagContainer.h"

class UDataTable;

class UFortNonPlayerConversationParticipantComponent : public UConversationParticipantComponent
{
public:
    static UClass* StaticClass();

    void InitializeServices();

    bool HasServiceData() const;

    FGameplayTag GetServiceProviderTag() const;

    TArray<const uint8*> GetSupportedSaleRows() const;

    FGameplayTag GetConversationEntryTag() const;

    FGameplayTag GetInteractorParticipantTag() const;

    FGameplayTag GetSelfParticipantTag() const;

    FGameplayTagContainer& GetSupportedServices() const;

    UDataTable*& GetServices() const;

    UDataTable*& GetSalesInventory() const;
};
