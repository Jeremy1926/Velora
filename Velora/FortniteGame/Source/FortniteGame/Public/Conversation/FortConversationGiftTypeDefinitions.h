#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"

class UFortControllerEffect;
class UFortControllerRequirement;

struct FConversationGiftTypeDefinition
{
    uint8 RequirementMatchPolicy;
    TArray<UFortControllerRequirement*> Requirements;
    TArray<UFortControllerEffect*> Effects;
    UObject* UINotificationType;
};

class UFortConversationGiftTypeDefinitions : public UDataAsset
{
public:
    static UClass* StaticClass();

    const TMap<FGameplayTag, FConversationGiftTypeDefinition>& GetGiftDefinitionsByGiftTag() const;
};
