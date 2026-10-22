#include "pch.h"
#include "Conversation/FortConversationTaskNode_ShowFutureStormCircle.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"

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
