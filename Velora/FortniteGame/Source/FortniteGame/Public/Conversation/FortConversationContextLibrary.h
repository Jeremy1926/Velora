#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ConversationContext.h"

class AActor;
class AFortPlayerController;
class UConversationParticipantComponent;

class UFortConversationContextLibrary : public UBlueprintFunctionLibrary
{
public:
    static AActor* GetParticipantActorNPC(const FConversationContext& Context);

    static AActor* GetParticipantActorPlayer(const FConversationContext& Context);

    static UConversationParticipantComponent* GetParticipantComponentNPC(const FConversationContext& Context);

    static AFortPlayerController* GetParticipantPlayerController(const FConversationContext& Context);
};
