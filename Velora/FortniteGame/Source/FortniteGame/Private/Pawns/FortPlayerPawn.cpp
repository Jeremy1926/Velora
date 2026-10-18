#include "pch.h"
#include "Pawns/FortPlayerPawn.h"
#include "Items/FortPickup.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortHealthSet.h"
#include "AI/FortAIPerceptionSystem.h"
#include "Player/FortPlayerControllerZone.h"
#include "Player/StatManager.h"
#include "FortFeedbackManager.h"
#include "FortGameDeathPenalty.h"
#include "FortGameMode.h"
#include "FortGameState.h"
#include "GameplayEffect.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

void AFortPlayerPawn::OnBlueprintPlace()
{
    static auto OnBlueprintPlace = []
    {
        auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

        return (void (*)(AFortPlayerPawn*))ClientBuild.Find("mov rcx, [*+0x*]; test rcx, rcx; jz *; call 0x*").Call().Get(TEXT("AFortPlayerPawn::OnBlueprintPlace"));
    }();

    OnBlueprintPlace(this);
}

void AFortPlayerPawn::ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    if (Pickup && !Pickup->IsPendingKill())
        Pickup->SetPickupTarget(this, InFlyTime, InStartDirection, bPlayPickupSound);
}

void AFortPlayerPawn::ServerReviveFromDBNO_Implementation(AController* EventInstigator)
{
    auto FortAbilitySystemComponent = Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());

    if (CanReviveFromDBNO(EventInstigator) && FortAbilitySystemComponent)
    {
        const bool bSelfRevive = EventInstigator == GetController();
        auto ReviveGameplayEffect = bSelfRevive ? GetSelfReviveGameplayEffect() : GetTeammateReviveGameplayEffect();
        auto SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(ReviveGameplayEffect, 0.f, FortAbilitySystemComponent->MakeEffectContext());

        if (SpecHandle.IsValid())
        {
            GetIsDBNO() = false;
            GetPlayedDying() = false;

            const float ReviveHealth = GetSetByCallerReviveHealth().GetValueAtLevel(0.f);

            if (ReviveHealth > 0.f)
            {
                float CurrentHealth = 0.f;

                if (auto HealthSet = GetHealthSet())
                    CurrentHealth = HealthSet->GetHealth().GetCurrentValue();

                SpecHandle.Data->SetSetByCallerMagnitude(FName(TEXT("ReviveHeal")), ReviveHealth - CurrentHealth);
            }

            UpdateDBNOInteractCollision();

            GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            static auto DBNOTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.DBNO")));
            static auto DBNOAthenaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.DBNOAthena")));

            StopPerformingNativeAction(DBNOTag);
            StopPerformingNativeAction(DBNOAthenaTag);
            UFortAIPerceptionSystem::ForceForgetEnemy(this);

            FGameplayTagContainer AbilityTagsToCancel;
            AbilityTagsToCancel.AddTag(DBNOTag);
            AbilityTagsToCancel.AddTag(DBNOAthenaTag);
            FortAbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);

            FGameplayEventData Payload;
            Payload.Instigator = EventInstigator;
            Payload.Target = this;
            FortAbilitySystemComponent->HandleGameplayEvent(GetEventReviveTag(), &Payload);

            if (auto FeedbackManager = AFortFeedbackManager::GetFeedbackManager(this))
            {
                FFortFeedbackHandle Handle{ nullptr, FName(TEXT("Resurrected")), false, false, EFortFeedbackBroadcastFilter::FFBF_None_Max };

                FeedbackManager->TriggerEvent(Handle, EventInstigator ? Cast<AFortPawn>(EventInstigator->GetPawn()) : nullptr, this, -1.f, false, true);
            }

            auto World = GetWorld();
            auto PlayerController = Cast<AFortPlayerControllerZone>(GetController());

            if (World && PlayerController)
            {
                PlayerController->OnPawnRevived(EventInstigator);
                PlayerController->ResetQuickBarToPrimary();

                auto GameState = Cast<AFortGameState>(World->GetGameState());

                if (GameState && GameState->GetGameplayState() != EFortGameplayState::EndOfZone)
                {
                    if (auto GameMode = Cast<AFortGameMode>(World->GetAuthGameMode()))
                    {
                        if (auto DeathPenalty = GameMode->GetDeathPenaltyData().LoadSynchronous())
                            PlayerController->ApplyDeathDurabilityPenalty(bSelfRevive ? DeathPenalty->GetSelfResurrectDurabilityPenalty() : DeathPenalty->GetTeammateResurrectDurabilityPenalty());
                    }
                }
            }

            if (!bSelfRevive && EventInstigator)
            {
                auto InstigatorController = Cast<AFortPlayerControllerZone>(EventInstigator);

                if (InstigatorController && InstigatorController->GetStatManager())
                {
                    if (auto Stat = InstigatorController->GetStatManager()->AddOrGetStat(FName(TEXT("TeammateRevives")), EStatRecordingPeriod::Campaign))
                        Stat->ModifyStat(1, EStatMod::Delta);
                }
            }
        }
    }
}

bool AFortPlayerPawn::CanReviveFromDBNO(AController* EventInstigator)
{
    static auto CanReviveFromDBNO = Scanner::At(Scanner::String("ResurrectHere").Last("lea rax, [0x*]").Target(TEXT("AFortPlayerControllerZone::ResurrectHere"))).Find("call [*+0x*]").Slot(TEXT("AFortPlayerPawn::CanReviveFromDBNO"));

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(AFortPlayerPawn*, AController*))(VTable + CanReviveFromDBNO))(this, EventInstigator);
}

void AFortPlayerPawn::UpdateDBNOInteractCollision()
{
    static auto UpdateDBNOInteractCollision = Scanner::Function<void (*)(AFortPlayerPawn*)>(Scanner::String(TEXT("DBNOInteractCollision")).Start().Get(TEXT("AFortPlayerPawn::UpdateDBNOInteractCollision")));

    UpdateDBNOInteractCollision(this);
}

TSubclassOf<UGameplayEffect>& AFortPlayerPawn::GetSelfReviveGameplayEffect()
{
    static auto SelfReviveGameplayEffect = GetClass()->FindPropertyByName(FName(TEXT("SelfReviveGameplayEffect")));

    return *SelfReviveGameplayEffect->ContainerPtrToValuePtr<TSubclassOf<UGameplayEffect>>(this);
}

TSubclassOf<UGameplayEffect>& AFortPlayerPawn::GetTeammateReviveGameplayEffect()
{
    static auto TeammateReviveGameplayEffect = GetClass()->FindPropertyByName(FName(TEXT("TeammateReviveGameplayEffect")));

    return *TeammateReviveGameplayEffect->ContainerPtrToValuePtr<TSubclassOf<UGameplayEffect>>(this);
}

FScalableFloat& AFortPlayerPawn::GetSetByCallerReviveHealth()
{
    static auto SetByCallerReviveHealth = GetClass()->FindPropertyByName(FName(TEXT("SetByCallerReviveHealth")));

    return *SetByCallerReviveHealth->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FGameplayTag& AFortPlayerPawn::GetEventReviveTag()
{
    static auto EventReviveTag = GetClass()->FindPropertyByName(FName(TEXT("EventReviveTag")));

    return *EventReviveTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}
