#include "pch.h"
#include "Conversation/FortConversationRequirement_HasService.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationRequirement_HasService::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationRequirement_HasService"));

    return Class;
}

EConversationRequirementResult UFortConversationRequirement_HasService::IsRequirementSatisfied_Implementation(const FConversationContext& Context) const
{
    static auto ServiceTag = StaticClass()->FindPropertyByName(FName(TEXT("ServiceTag")));

    auto NPCConversationParticipantComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));

    if (NPCConversationParticipantComponent && NPCConversationParticipantComponent->GetSupportedServices().HasTag(ServiceTag->GetPropertyValue_InContainer<FGameplayTag>(this)))
        return EConversationRequirementResult::Passed;

    return EConversationRequirementResult::FailedAndHidden;
}
