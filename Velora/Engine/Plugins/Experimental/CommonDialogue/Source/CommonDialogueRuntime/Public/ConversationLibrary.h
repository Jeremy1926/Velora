#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"

class AActor;
class UConversationInstance;
struct FFrame;

class UConversationLibrary : public UBlueprintFunctionLibrary
{
public:
    static UConversationInstance* StartConversation(FGameplayTag ConversationEntryTag, AActor* Instigator, FGameplayTag InstigatorTag, AActor* Target, FGameplayTag TargetTag);

    void execStartConversation(FFrame& Stack, void* const Z_Param__Result);
};
