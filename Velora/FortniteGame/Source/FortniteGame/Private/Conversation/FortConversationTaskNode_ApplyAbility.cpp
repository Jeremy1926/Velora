#include "pch.h"
#include "Conversation/FortConversationTaskNode_ApplyAbility.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Pawns/FortPlayerPawn.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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

EPreventAbilityUseReason UFortConversationTaskNode_ApplyAbility::CheckPlayerFacingUsageRequirements(const FConversationContext& Context, const AFortPlayerPawn& PlayerPawn, int32 Price) const
{
    static auto CheckPlayerFacingUsageRequirements = Anchor::Function<EPreventAbilityUseReason (*)(const UFortConversationTaskNode_ApplyAbility*, const FConversationContext&, const AFortPlayerPawn&, int32)>(TEXT("UFortConversationTaskNode_ApplyAbility::CheckPlayerFacingUsageRequirements"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_ApplyAbility"), 0)
        .Find("call 0x*; cmp al, 0x3")
        .Target()
        .Get();

    return CheckPlayerFacingUsageRequirements(this, Context, PlayerPawn, Price);
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

void UFortConversationTaskNode_ApplyAbility::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    const TOptional<int32> Price = FindServicePrice(GetPricingKey(), 0);

    if (Price.IsSet())
        InOutExtraData.Add({ FString(TEXT("cost")), FString::FromInt(Price.GetValue()) });

    auto PlayerController = Cast<AFortPlayerControllerAthena>(UFortConversationContextLibrary::GetParticipantActorPlayer(Context));
    auto PlayerPawn = PlayerController ? Cast<AFortPlayerPawn>(PlayerController->GetPawn()) : nullptr;

    if (!PlayerPawn)
        return;

    const EPreventAbilityUseReason Reason = CheckPlayerFacingUsageRequirements(Context, *PlayerPawn, Price.GetValue());

    if (Reason != EPreventAbilityUseReason::None)
        InOutExtraData.Add({ FString(TEXT("cannotpurchasereason")), FString::FromInt((int32)Reason) });
}
