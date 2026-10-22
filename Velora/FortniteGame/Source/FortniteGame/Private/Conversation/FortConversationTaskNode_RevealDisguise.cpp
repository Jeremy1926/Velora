#include "pch.h"
#include "Conversation/FortConversationTaskNode_RevealDisguise.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Components/FortPawnComponent_Disguise.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_RevealDisguise::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_RevealDisguise"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_RevealDisguise::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto NPCActor = UFortConversationContextLibrary::GetParticipantActorNPC(Context);
    auto Disguise = NPCActor ? NPCActor->FindComponentByClass<UFortPawnComponent_Disguise>() : nullptr;

    if (!Disguise)
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    Disguise->RevealDisguise(PlayerController ? (const AActor*)PlayerController->GetMyFortPawn() : nullptr);

    return FConversationTaskResult::AdvanceConversation();
}
