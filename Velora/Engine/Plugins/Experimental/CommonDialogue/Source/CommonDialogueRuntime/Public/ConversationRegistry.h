#pragma once

#include "UObject/Object.h"
#include "Containers/Map.h"
#include "ConversationTypes.h"

class UConversationNode;
class UWorld;

struct FConversationEntryList
{
    TArray<FGuid> DestinationList;
};

class UConversationRegistry : public UObject
{
public:
    static UClass* StaticClass();

    static UConversationRegistry* GetFromWorld(const UWorld* World);

    UConversationNode* GetRuntimeNodeFromGUID(const FGuid& NodeGUID) const;

    TArray<FGuid> GetEntryPointGUIDs(FGameplayTag EntryPoint) const;

    TArray<FGuid> GetOutputLinkGUIDs(FGameplayTag EntryPoint) const;

    TArray<FGuid> GetOutputLinkGUIDs(const TArray<FGuid>& SourceGUIDs) const;

    const TMap<FGameplayTag, FConversationEntryList>& GetEntryMap() const;

    const TMap<FGuid, UConversationNode*>& GetReachableNodeMap() const;
};
