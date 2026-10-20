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

static int32_t OnPawnRevivedSlot()
{
    static auto OnPawnRevived = []
    {
        auto ClientOnPawnRevived = Scanner::Event(TEXT("ClientOnPawnRevived")).Get(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"));

        return Anchor::Slot(TEXT("AFortPlayerControllerZone::OnPawnRevived"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"), [&](const Anchor::FCode& Entry)
            {
                auto Call = Entry.Find("call 0x*");

                for (int32 Index = 2; Call.Get() && Call.Target() != ClientOnPawnRevived; Index++)
                    Call = Entry.Find("call 0x*", Index);

                return Call.Get() != nullptr;
            })
            .Get();
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

DETOUR(AFortPlayerControllerZone, OnPawnDied, Anchor::Hook((void*)+[](AFortPlayerControllerZone* PlayerController, AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
    {
        PlayerController->OnPawnDied(KilledPawn, Damage, InTags, EffectContext, EventInstigator, DamageCauser, DBNOFinisher);
    })
    .String(TEXT("Deaths"))
    .Start()
    .Find("movaps xmm6, xmm*")
    .Is("movaps xmm6, xmm2")
    .Start()
    .Or()
    .String(TEXT("Deaths"))
    .Start()
    .Find("movaps xmm6, xmm*")
    .Is("movaps xmm6, xmm1")
    .Start()
    .With((void*)+[](AFortPlayerControllerZone* PlayerController, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser)
    {
        PlayerController->OnPawnDied(PlayerController->GetMyFortPawn(), Damage, InTags, EffectContext, EventInstigator, DamageCauser, nullptr);
    })
    .Optional()
    .Inline())
void AFortPlayerControllerZone::OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
{
    static auto bKilledPawn = Scanner::String(TEXT("Deaths")).Start().Find("movaps xmm6, xmm*").Is("movaps xmm6, xmm2");

    if (bKilledPawn)
        ((void (*)(AFortPlayerControllerZone*, AFortPlayerPawn*, float, const FGameplayTagContainer&, const FGameplayEffectContextHandle&, AController*, AActor*, AController*))DETOUR_ORIGINAL(AFortPlayerControllerZone, OnPawnDied))(this, KilledPawn, Damage, InTags, EffectContext, EventInstigator, DamageCauser, DBNOFinisher);
    else
        ((void (*)(AFortPlayerControllerZone*, float, const FGameplayTagContainer&, const FGameplayEffectContextHandle&, AController*, AActor*))DETOUR_ORIGINAL(AFortPlayerControllerZone, OnPawnDied))(this, Damage, InTags, EffectContext, EventInstigator, DamageCauser);

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
    static auto bApplyDeathDurabilityPenalty = Scanner::At(Scanner::Virtual<const void*>(this, OnPawnRevivedSlot(), sizeof(void*))).Is("mov r11, rsp; push r13; sub rsp, *; mov r13, rcx; movaps [rsp+*], xmm7; mov rcx, [rcx+*]; movaps xmm7, xmm1");

    if (bApplyDeathDurabilityPenalty)
        Scanner::Virtual<void (*)(AFortPlayerControllerZone*, float)>(this, OnPawnRevivedSlot(), sizeof(void*))(this, DurabilityPenalty);
}
