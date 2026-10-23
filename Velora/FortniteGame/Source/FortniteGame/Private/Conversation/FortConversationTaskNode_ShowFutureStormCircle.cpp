#include "pch.h"
#include "Conversation/FortConversationTaskNode_ShowFutureStormCircle.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_ShowFutureStormCircle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_ShowFutureStormCircle"));

    return Class;
}

UClass* UFortConversationTaskNode_ShowFutureStormCircle::GetGrantedAbility() const
{
    static auto GrantedAbility = StaticClass()->FindPropertyByName(FName(TEXT("GrantedAbility")));

    return *GrantedAbility->ContainerPtrToValuePtr<UClass*>(this);
}

int32 UFortConversationTaskNode_ShowFutureStormCircle::GetLevel() const
{
    static auto Level = StaticClass()->FindPropertyByName(FName(TEXT("Level")));

    return *Level->ContainerPtrToValuePtr<int32>(this);
}

EPreventUseStormCircleServiceReason UFortConversationTaskNode_ShowFutureStormCircle::CheckPlayerFacingUsageRequirements(const FConversationContext& Context, const AFortPlayerPawn& PlayerPawn, int32 Price) const
{
    static auto CheckPlayerFacingUsageRequirements = Anchor::Function<EPreventUseStormCircleServiceReason (*)(const UFortConversationTaskNode_ShowFutureStormCircle*, const FConversationContext&, const AFortPlayerPawn&, int32)>(TEXT("UFortConversationTaskNode_ShowFutureStormCircle::CheckPlayerFacingUsageRequirements"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_ShowFutureStormCircle"), 0)
        .Find("call 0x*; cmp al, 0x3")
        .Target()
        .Get();

    return CheckPlayerFacingUsageRequirements(this, Context, PlayerPawn, Price);
}

FConversationTaskResult UFortConversationTaskNode_ShowFutureStormCircle::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto AbilitySystemComponent = PlayerPawn ? PlayerPawn->GetAbilitySystemComponent() : nullptr;
    auto GrantedAbility = GetGrantedAbility();

    if (!AbilitySystemComponent || !GrantedAbility || !PayPrice(Context, FindServicePrice(FName(TEXT("showfuturestormcircle")), 0)))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    GetForecastPlayers().push_back({ PlayerController, AbilitySystemComponent->GiveAbilityAndActivateOnce(FGameplayAbilitySpec(GrantedAbility->GetDefaultObject<UGameplayAbility>(), GetLevel())) });
    AFortSafeZoneIndicatorFuture::SetWhitelistedPlayer(PlayerController, true);

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_ShowFutureStormCircle::HandleSafeZoneUpdated()
{
    for (auto& Entry : GetForecastPlayers())
    {
        auto PlayerController = Entry.first.Get();
        auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
        auto AbilitySystemComponent = PlayerPawn ? PlayerPawn->GetAbilitySystemComponent() : nullptr;

        if (PlayerController)
            AFortSafeZoneIndicatorFuture::SetWhitelistedPlayer(PlayerController, false);

        if (AbilitySystemComponent)
            AbilitySystemComponent->ClearAbility(Entry.second);
    }

    GetForecastPlayers().clear();
}

std::vector<std::pair<TWeakObjectPtr<AFortPlayerController>, FGameplayAbilitySpecHandle>>& UFortConversationTaskNode_ShowFutureStormCircle::GetForecastPlayers()
{
    static std::vector<std::pair<TWeakObjectPtr<AFortPlayerController>, FGameplayAbilitySpecHandle>> ForecastPlayers;

    return ForecastPlayers;
}

void UFortConversationTaskNode_ShowFutureStormCircle::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    const TOptional<int32> Price = FindServicePrice(FName(TEXT("showfuturestormcircle")), 0);

    if (Price.IsSet())
        InOutExtraData.Add({ FString(TEXT("cost")), FString::FromInt(Price.GetValue()) });

    auto PlayerController = Cast<AFortPlayerControllerAthena>(UFortConversationContextLibrary::GetParticipantActorPlayer(Context));
    auto PlayerPawn = PlayerController ? Cast<AFortPlayerPawn>(PlayerController->GetPawn()) : nullptr;

    if (!PlayerPawn)
        return;

    const EPreventUseStormCircleServiceReason Reason = CheckPlayerFacingUsageRequirements(Context, *PlayerPawn, Price.GetValue());

    if (Reason == EPreventUseStormCircleServiceReason::None)
        return;

    static auto PreventUseStormCircleServiceReason = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EPreventUseStormCircleServiceReason"));

    InOutExtraData.Add({ FString(TEXT("cannotpurchasereason")), PreventUseStormCircleServiceReason ? PreventUseStormCircleServiceReason->GetNameStringByValue((int64)Reason) : FString() });
}
