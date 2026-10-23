#include "pch.h"
#include "Conversation/FortConversationTaskNode_HireNPC.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "ConversationParticipantComponent.h"
#include "Pawns/FortPawnComponent_Convert.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_HireNPC::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_HireNPC"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_HireNPC::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto ConvertComponent = PlayerPawn ? PlayerPawn->FindComponentByClass<UFortPawnComponent_Convert>() : nullptr;
    auto NPCComponent = UFortConversationContextLibrary::GetParticipantComponentNPC(Context);
    auto NPCPawn = Cast<AFortPawn>(NPCComponent ? NPCComponent->GetOwner() : nullptr);

    if (!ConvertComponent || !NPCPawn || !PayPrice(Context, FindServicePrice(FName(TEXT("HireNPC")), 0)))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    ConvertComponent->ConvertPawn(NPCPawn);

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_HireNPC::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    if (!Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context)))
        return;

    const TOptional<int32> HirePrice = FindServicePrice(FName(TEXT("HireNPC")), 0);

    if (HirePrice.IsSet())
        InOutExtraData.Add({ FString(TEXT("cost")), FString::FromInt(HirePrice.GetValue()) });
}
