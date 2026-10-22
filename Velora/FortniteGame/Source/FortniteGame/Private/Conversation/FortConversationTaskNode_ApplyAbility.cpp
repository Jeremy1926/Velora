#include "pch.h"
#include "Conversation/FortConversationTaskNode_ApplyAbility.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortConversationTaskNode_ApplyAbility::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_ApplyAbility"));

    return Class;
}

UClass* UFortConversationTaskNode_ApplyAbility::GetGrantedAbility() const
{
    static auto GrantedAbility = StaticClass()->FindPropertyByName(FName(TEXT("GrantedAbility")));

    return *GrantedAbility->ContainerPtrToValuePtr<UClass*>(this);
}

int32 UFortConversationTaskNode_ApplyAbility::GetLevel() const
{
    static auto Level = StaticClass()->FindPropertyByName(FName(TEXT("Level")));

    return *Level->ContainerPtrToValuePtr<int32>(this);
}

bool UFortConversationTaskNode_ApplyAbility::GetbActivateImmediatelyOnGranting() const
{
    static auto bActivateImmediatelyOnGranting = StaticClass()->FindPropertyByName(FName(TEXT("bActivateImmediatelyOnGranting")));

    return *bActivateImmediatelyOnGranting->ContainerPtrToValuePtr<bool>(this);
}

FName UFortConversationTaskNode_ApplyAbility::GetPricingKey() const
{
    static auto PricingKey = StaticClass()->FindPropertyByName(FName(TEXT("PricingKey")));

    return *PricingKey->ContainerPtrToValuePtr<FName>(this);
}

FConversationTaskResult UFortConversationTaskNode_ApplyAbility::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto AbilitySystemComponent = PlayerPawn ? PlayerPawn->GetAbilitySystemComponent() : nullptr;
    auto GrantedAbility = GetGrantedAbility();

    if (!AbilitySystemComponent || !GrantedAbility || !PayPrice(Context, FindServicePrice(GetPricingKey(), 0)))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const FGameplayAbilitySpec Spec(GrantedAbility->GetDefaultObject<UGameplayAbility>(), GetLevel());

    if (GetbActivateImmediatelyOnGranting())
        AbilitySystemComponent->GiveAbilityAndActivateOnce(Spec);
    else
        AbilitySystemComponent->GiveAbility(Spec);

    return FConversationTaskResult::AdvanceConversation();
}
