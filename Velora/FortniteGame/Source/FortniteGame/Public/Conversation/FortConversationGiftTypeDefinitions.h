#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"

class UFortControllerEffect;
class UFortControllerRequirement;
class UScriptStruct;

struct FConversationGiftTypeDefinition
{
    static UScriptStruct* StaticStruct();

    uint8 GetRequirementMatchPolicy() const;

    const TArray<UFortControllerRequirement*>& GetRequirements() const;

    const TArray<UFortControllerEffect*>& GetEffects() const;
};

template<>
class TMap<FGameplayTag, FConversationGiftTypeDefinition>
{
public:
    const FConversationGiftTypeDefinition* Find(const FGameplayTag& Key) const;

private:
    TArray<uint8> Elements;
    TBitArray<> AllocationFlags;
};

class UFortConversationGiftTypeDefinitions : public UDataAsset
{
public:
    static UClass* StaticClass();

    const TMap<FGameplayTag, FConversationGiftTypeDefinition>& GetGiftDefinitionsByGiftTag() const;
};
