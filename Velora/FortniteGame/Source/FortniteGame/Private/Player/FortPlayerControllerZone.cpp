#include "pch.h"
#include "Player/FortPlayerControllerZone.h"
#include "Player/FortPlayerState.h"
#include "Pawns/FortPlayerPawn.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FFortPlayerDeathReport::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerDeathReport"));

    return Struct;
}

AFortPlayerState* FFortPlayerDeathReport::GetKillerPlayerState() const
{
    static auto KillerPlayerState = StaticStruct()->FindPropertyByName(FName(TEXT("KillerPlayerState")));

    return *KillerPlayerState->ContainerPtrToValuePtr<AFortPlayerState*>(this);
}

AFortPawn* FFortPlayerDeathReport::GetKillerPawn() const
{
    static auto KillerPawn = StaticStruct()->FindPropertyByName(FName(TEXT("KillerPawn")));

    return *KillerPawn->ContainerPtrToValuePtr<AFortPawn*>(this);
}

UFortWeaponItemDefinition* FFortPlayerDeathReport::GetKillerWeapon() const
{
    static auto KillerWeapon = StaticStruct()->FindPropertyByName(FName(TEXT("KillerWeapon")));

    return *KillerWeapon->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
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
    static auto ServerRestartPlayer = Anchor::VirtualFunction<void (*)(AFortPlayerControllerZone*)>(TEXT("AFortPlayerControllerZone::ServerRestartPlayer_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer"))
        .Find("jmp [rax+*]")
        .Get(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

    ServerRestartPlayer(this);
}

void AFortPlayerControllerZone::OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
{
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
        .Inline());

    static auto bKilledPawn = Anchor::Exists()
        .String(TEXT("Deaths"))
        .Start()
        .Find("movaps xmm6, xmm*")
        .Is("movaps xmm6, xmm2")
        .Get();

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
    static auto OnPawnRevived = Anchor::VirtualFunction<void (*)(AFortPlayerControllerZone*, AController*)>(TEXT("AFortPlayerControllerZone::OnPawnRevived"))
        .Event(TEXT("ClientOnPawnRevived"))
        .Log(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Get();

    OnPawnRevived(this, EventInstigator);
}

void AFortPlayerControllerZone::ApplyDeathDurabilityPenalty(float DurabilityPenalty)
{
    static auto bApplyDeathDurabilityPenalty = Anchor::Exists()
        .Event(TEXT("ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .SlotShift(this, sizeof(void*))
        .Is("mov r11, rsp; push r13; sub rsp, *; mov r13, rcx; movaps [rsp+*], xmm7; mov rcx, [rcx+*]; movaps xmm7, xmm1")
        .Get();

    static auto ApplyDeathDurabilityPenalty = Anchor::VirtualFunction<void (*)(AFortPlayerControllerZone*, float)>(TEXT("AFortPlayerControllerZone::OnPawnRevived"))
        .Event(TEXT("ClientOnPawnRevived"))
        .Log(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Get(sizeof(void*));

    if (bApplyDeathDurabilityPenalty)
        ApplyDeathDurabilityPenalty(this, DurabilityPenalty);
}

FFortPlayerDeathReport& AFortPlayerControllerZone::GetPlayerDeathReport()
{
    static auto PlayerDeathReport = GetClass()->FindPropertyByName(FName(TEXT("PlayerDeathReport")));

    return *PlayerDeathReport->ContainerPtrToValuePtr<FFortPlayerDeathReport>(this);
}
