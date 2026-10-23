#include "pch.h"
#include "Conversation/FortConversationTaskNode_ApplyDataDrivenEffect.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "AI/FortAthenaAIBotController.h"
#include "Pawns/FortPawn.h"
#include "Player/FortControllerEffect.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_ApplyDataDrivenEffect::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_ApplyDataDrivenEffect"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_ApplyDataDrivenEffect::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto ParticipantPawn = Cast<AFortPawn>(Context.GetParticipantActor(GetParticipantID()));
    auto TargetController = ParticipantPawn ? ParticipantPawn->GetController() : nullptr;
    auto NPCComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));

    FEffectRequestContext RequestContext;
    RequestContext.GetInstigatingController() = NPCComponent ? (AController*)NPCComponent->GetBotControllerOwner() : nullptr;
    RequestContext.SetInstigatingActor(UFortConversationContextLibrary::GetParticipantActorNPC(Context));

    for (auto Effect : GetEffectsToApply())
    {
        if (Effect && TargetController)
            Effect->ApplyEffect(TargetController, RequestContext);
    }

    return FConversationTaskResult::AdvanceConversation();
}

FGameplayTag UFortConversationTaskNode_ApplyDataDrivenEffect::GetParticipantID() const
{
    static auto ParticipantID = StaticClass()->FindPropertyByName(FName(TEXT("ParticipantID")));

    return ParticipantID->GetPropertyValue_InContainer<FGameplayTag>(this);
}

const TArray<UFortControllerEffect*>& UFortConversationTaskNode_ApplyDataDrivenEffect::GetEffectsToApply() const
{
    static auto EffectsToApply = StaticClass()->FindPropertyByName(FName(TEXT("EffectsToApply")));

    return *EffectsToApply->ContainerPtrToValuePtr<TArray<UFortControllerEffect*>>(this);
}
