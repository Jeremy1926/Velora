#include "pch.h"
#include "Player/FortPlayerControllerZone.h"
#include "Player/FortPlayerState.h"
#include "Pawns/FortPlayerPawn.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static void* OnPawnDied = []() -> void*
{
    auto OnPawnDied = Scanner::String(TEXT("Deaths")).Start();
    auto DamageRegister = OnPawnDied.Find("movaps xmm6, xmm*");

    if (DamageRegister.Is("movaps xmm6, xmm2"))
    {
        return Detour(TEXT("AFortPlayerControllerZone::OnPawnDied"), (void*)OnPawnDied.Get(), (void*)+[](AFortPlayerControllerZone* PlayerController, AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
        {
            PlayerController->OnPawnDied(KilledPawn, Damage, InTags, EffectContext, EventInstigator, DamageCauser, DBNOFinisher);
        });
    }

    if (DamageRegister.Is("movaps xmm6, xmm1"))
    {
        return Detour(TEXT("AFortPlayerControllerZone::OnPawnDied"), (void*)OnPawnDied.Get(), (void*)+[](AFortPlayerControllerZone* PlayerController, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser)
        {
            PlayerController->OnPawnDied(PlayerController->GetMyFortPawn(), Damage, InTags, EffectContext, EventInstigator, DamageCauser, nullptr);
        });
    }

    return nullptr;
}();

static int32_t OnPawnRevivedSlot()
{
    static auto OnPawnRevived = []
    {
        auto ClientOnPawnRevived = Scanner::Event(TEXT("ClientOnPawnRevived")).Get(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

        return Scanner::Slot(TEXT("AFortPlayerControllerZone::OnPawnRevived"), VTable, [&](const Scanner::FCode& Entry)
        {
            auto Call = Entry.Find("call 0x*");

            for (int32 Index = 2; Call.Get() && Call.Target() != ClientOnPawnRevived; Index++)
                Call = Entry.Find("call 0x*", Index);

            return Call.Get() != nullptr;
        });
    }();

    return OnPawnRevived;
}

UClass* AFortPlayerControllerZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerZone"));

    return Class;
}

void AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation(APawn* P)
{
    Super::ServerAcknowledgePossession_Implementation(P);
}

void AFortPlayerControllerZone::ServerRestartPlayer_Implementation()
{
    static auto ServerRestartPlayer = Scanner::Virtual<void (*)(AFortPlayerControllerZone*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone")), Scanner::Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer")).Find("jmp [rax+*]").Slot(TEXT("AFortPlayerControllerZone::ServerRestartPlayer_Implementation")));

    ServerRestartPlayer(this);
}

void AFortPlayerControllerZone::OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
{
    static auto bKilledPawn = Scanner::String(TEXT("Deaths")).Start().Find("movaps xmm6, xmm*").Is("movaps xmm6, xmm2");

    if (bKilledPawn)
        ((void (*)(AFortPlayerControllerZone*, AFortPlayerPawn*, float, const FGameplayTagContainer&, const FGameplayEffectContextHandle&, AController*, AActor*, AController*))::OnPawnDied)(this, KilledPawn, Damage, InTags, EffectContext, EventInstigator, DamageCauser, DBNOFinisher);
    else
        ((void (*)(AFortPlayerControllerZone*, float, const FGameplayTagContainer&, const FGameplayEffectContextHandle&, AController*, AActor*))::OnPawnDied)(this, Damage, InTags, EffectContext, EventInstigator, DamageCauser);

    static auto EarnedElimination = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.EarnedElimination")), false);

    if (!EarnedElimination.IsValid() || !EventInstigator || EventInstigator == this)
        return;

    auto PlayerState = Cast<AFortPlayerState>(EventInstigator->GetPlayerState());
    auto AbilitySystemComponent = PlayerState ? PlayerState->GetAbilitySystemComponent() : nullptr;

    if (!AbilitySystemComponent)
        return;

    FGameplayEventData Payload;
    Payload.Instigator = EventInstigator;
    Payload.Target = KilledPawn;
    AbilitySystemComponent->HandleGameplayEvent(EarnedElimination, &Payload);
}

void AFortPlayerControllerZone::OnPawnRevived(AController* EventInstigator)
{
    Scanner::Virtual<void (*)(AFortPlayerControllerZone*, AController*)>(this, OnPawnRevivedSlot())(this, EventInstigator);
}

void AFortPlayerControllerZone::ApplyDeathDurabilityPenalty(float DurabilityPenalty)
{
    Scanner::Virtual<void (*)(AFortPlayerControllerZone*, float)>(this, OnPawnRevivedSlot(), sizeof(void*))(this, DurabilityPenalty);
}
