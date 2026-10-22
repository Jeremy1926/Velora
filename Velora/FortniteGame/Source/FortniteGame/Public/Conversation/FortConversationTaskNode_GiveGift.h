#pragma once

#include "Conversation/FortConversationTaskNode.h"

class AFortPlayerController;
class UFortNPCConversationParticipantComponent;

class UFortConversationTaskNode_GiveGift : public UFortConversationTaskNode
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    static void GiveGift(UFortNPCConversationParticipantComponent* NPCComponent, AFortPlayerController* PlayerController);

    static void GiveGiftItems(UFortNPCConversationParticipantComponent* NPCComponent, AFortPlayerController* PlayerController);
};
