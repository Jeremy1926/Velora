#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class AActor;
class AFortPlayerPawn;

class UFortConversationTaskNode_DuelNPC : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;

    FConversationTaskResult ExecuteChallengeTaskNode(const FConversationContext& Context) const;

    void SetChallenger(AActor* NonParticipantPawn, AFortPlayerPawn* Challenger) const;
};
