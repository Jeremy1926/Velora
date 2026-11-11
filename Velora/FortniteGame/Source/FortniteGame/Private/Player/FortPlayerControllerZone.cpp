#include "pch.h"
#include "Player/FortPlayerControllerZone.h"
#include "Player/FortPlayerState.h"
#include "Player/FortPlayerStateZone.h"
#include "Pawns/FortPlayerPawn.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPoiManager.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "FortGameState.h"
#include "FortGlobals.h"
#include "FortRuntimeOptions.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"
#include "Config.h"

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
    static auto bWeakKillerPawn = KillerPawn->IsWeakObjectProperty();

    if (bWeakKillerPawn)
        return (AFortPawn*)KillerPawn->ContainerPtrToValuePtr<FWeakObjectPtr>(this)->Get();

    return *KillerPawn->ContainerPtrToValuePtr<AFortPawn*>(this);
}

UFortWeaponItemDefinition* FFortPlayerDeathReport::GetKillerWeapon() const
{
    static auto KillerWeapon = StaticStruct()->FindPropertyByName(FName(TEXT("KillerWeapon")));

    return KillerWeapon ? *KillerWeapon->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this) : nullptr;
}

UClass* AFortPlayerControllerZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerZone"));

    return Class;
}

FSimpleMulticastDelegate& AFortPlayerControllerZone::GetOnPlayerPlaceChanged()
{
    static auto OnPlayerPlaceChanged = Anchor::Offset(TEXT("AFortPlayerControllerZone::OnPlayerPlaceChanged"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnRep_Place"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov *, [*+0x*]; test *, *; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("lea rcx, [*+0x*]; call 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnRep_Place"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov *, [*+0x*]; test *, *; jz 0x*")
        .Find("lea rcx, [*+0x*]; call 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnRep_Place"))
        .Find("jmp 0x*")
        .Target()
        .Find("test al, al; jz 0x*; add rbx, 0x*")
        .Find("add rbx, 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnRep_Place"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov *, [*+0x*]; test *, *; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("test al, al; jz 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("lea rcx, [*+0x*]; call 0x*")
        .Get();

    return *(FSimpleMulticastDelegate*)((uint8*)this + OnPlayerPlaceChanged);
}

decltype(AFortPlayerControllerZone::GameServerAcknowledgePossession) AFortPlayerControllerZone::GameServerAcknowledgePossession = nullptr;

void AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation(APawn* P)
{
    auto bInitializeGameplay = P && !GetAcknowledgedPawn();

    Super::ServerAcknowledgePossession_Implementation(P);

    if (Config::bLogReplication)
        UE_LOG(LogVelora, Log, TEXT("Acknowledged pawn %s for %s, initialize abilities %d"), P ? *P->GetName() : TEXT("none"), *GetName(), (int32)(bInitializeGameplay && UFortGlobals::HasInitializePlayerGameplayAbilities()));

    if (bInitializeGameplay && UFortGlobals::HasInitializePlayerGameplayAbilities())
    {
        static auto AbilitySystemInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemInterface"));

        if (auto PlayerState = GetPlayerState())
            UFortGlobals::InitializePlayerGameplayAbilities((IAbilitySystemInterface*)PlayerState->GetInterfaceAddress(AbilitySystemInterface));
    }

    if (GameServerAcknowledgePossession)
        (this->*GameServerAcknowledgePossession)(P);
}

void AFortPlayerControllerZone::ServerRestartPlayer_Implementation()
{
    static auto ServerRestartPlayer = Anchor::VirtualFunction<void (*)(AFortPlayerControllerZone*)>(TEXT("AFortPlayerControllerZone::ServerRestartPlayer_Implementation"))
        .Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer"))
        .Find("jmp [rax+*]")
        .Get(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

    ServerRestartPlayer(this);
}

void AFortPlayerControllerZone::ServerSpectateActor_Implementation(AActor* NewViewTarget, bool bAllowStateChange)
{
    DETOUR(AFortPlayerControllerZone, ServerSpectateActor_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerZone:ServerSpectateActor"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]", 2)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"), "ret*")
        .Optional()
        .InChildVTablesCounted(StaticClass()));

    if (!bAllowStateChange && GetStateName() != FName(TEXT("Spectating")))
    {
        UE_LOG(LogVelora, Log, TEXT("AFortPlayerControllerZone::ServerSpectateActor_Implementation Aborted, unable to set Player Controler [%s]'s Spectate Target to [%s] because state change is NOT allowed."), *GetName(), NewViewTarget ? *NewViewTarget->GetName() : TEXT("NULL"));
        return;
    }

    if (!CanSpectateActor(NewViewTarget))
    {
        UE_LOG(LogVelora, Log, TEXT("AFortPlayerControllerZone::ServerSpectateActor_Implementation Aborted, unable to set Player Controler [%s]'s Spectate Target to [%s] because CanSpectateActor() failed."), *GetName(), NewViewTarget ? *NewViewTarget->GetName() : TEXT("NULL"));
        return;
    }

    auto FortPlayerState = Cast<AFortPlayerStateZone>(GetPlayerState());

    if (!FortPlayerState || !NewViewTarget)
        return;

    if (ShouldSuicideOnSpectate(NewViewTarget))
    {
        UE_LOG(LogVelora, Warning, TEXT("%s used ServerSpectateActor() while still alive, requesting suicide!"), *GetName());
        ServerSuicide_Implementation();
    }

    AFortPlayerStateZone* TargetPlayerState = nullptr;

    if (auto ViewPawn = Cast<APawn>(NewViewTarget))
    {
        TargetPlayerState = Cast<AFortPlayerStateZone>(ViewPawn->GetPlayerState());

        auto PlayerPawn = TargetPlayerState ? nullptr : Cast<AFortPlayerPawn>(ViewPawn);

        if (PlayerPawn && PlayerPawn->GetCurrentVehicle())
        {
            auto VehicleInterface = PlayerPawn->GetVehicleInterface();

            if (auto VehicleController = VehicleInterface ? VehicleInterface->GetVehicleController(PlayerPawn) : nullptr)
                TargetPlayerState = Cast<AFortPlayerStateZone>(VehicleController->GetPlayerState());

            if (auto VehiclePawn = Cast<AFortPlayerPawnAthena>(PlayerPawn->GetCurrentVehicle()))
                NewViewTarget = VehiclePawn;
        }

        if (TargetPlayerState && UFortRuntimeOptions::Get().GetEnableSpectatorUpdates())
            FortPlayerState->BeginSpectating(TargetPlayerState);
    }

    ChangeState(FName(TEXT("Spectating")));
    ClientGotoState(FName(TEXT("Spectating")));
    SetViewTargetAndNotifyClient(NewViewTarget, FViewTargetTransitionParams());

    UE_LOG(LogVelora, Log, TEXT("[Spectate] %s now spectating %s (player state %s, spectator updates %d)"), *GetName(), *NewViewTarget->GetName(), TargetPlayerState ? *TargetPlayerState->GetName() : TEXT("none"), (int32)UFortRuntimeOptions::Get().GetEnableSpectatorUpdates());

    if (auto World = GetWorld())
    {
        if (auto GameState = Cast<AFortGameState>(World->GetGameState()))
        {
            if (auto PoiManager = GameState->GetPoiManager())
                PoiManager->CheckPlayerControllerWithPoiGrid(this);
        }
    }
}

bool AFortPlayerControllerZone::CanSpectateActor(AActor* InActor)
{
    static auto CanSpectateActor = Anchor::VirtualFunction<bool (*)(AFortPlayerControllerZone*, AActor*)>(TEXT("AFortPlayerControllerZone::CanSpectateActor"))
        .String("AFortPlayerControllerZone::FollowPlayer")
        .Start()
        .Find("call [rax+0x*]; test al, al; jnz 0x*")
        .Get();

    return CanSpectateActor(this, InActor);
}

bool AFortPlayerControllerZone::ShouldSuicideOnSpectate(AActor* NewViewTarget)
{
    static auto ShouldSuicideOnSpectate = Anchor::VirtualFunction<bool (*)(AFortPlayerControllerZone*, AActor*)>(TEXT("AFortPlayerControllerZone::ShouldSuicideOnSpectate"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateZone:IsPlayerDead"))
        .Find("call [rax+0x*]")
        .VirtualSlotCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"), "mov rax, [rcx]; ")
        .Get();

    return ShouldSuicideOnSpectate(this, NewViewTarget);
}

void AFortPlayerControllerZone::StartSpectateAfterDeathTimer()
{
    DETOUR(AFortPlayerControllerZone, StartSpectateAfterDeathTimer, Anchor::Hook()
        .If(DETOUR_ORIGINAL(AFortPlayerControllerZone, ServerSpectateActor_Implementation) != nullptr)
        .String(TEXT("Deaths"))
        .Find("add rcx, 0x4; cmp rcx, rdx; jnz 0x*; mov rcx, r*; call 0x*")
        .Find("call 0x*")
        .Optional()
        .AtCall());

    auto PlayerToSpectateOnDeath = GetPlayerToSpectateOnDeath();

    UE_LOG(LogVelora, Log, TEXT("[Spectate] %s died, starting the spectate after death timer (preferred target %s)"), *GetName(), PlayerToSpectateOnDeath ? *PlayerToSpectateOnDeath->GetName() : TEXT("none"));

    Original();
}

APawn* AFortPlayerControllerZone::GetPlayerToSpectateOnDeath() const
{
    static auto PlayerToSpectateOnDeath = StaticClass()->FindPropertyByName(FName(TEXT("PlayerToSpectateOnDeath")));

    return *PlayerToSpectateOnDeath->ContainerPtrToValuePtr<APawn*>(this);
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
        .Or()
        .Event(TEXT("ClientOnPawnRevived"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
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
        .Or()
        .Event(TEXT("ClientOnPawnRevived"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .SlotShift(this, sizeof(void*))
        .Is("mov r11, rsp; push r13; sub rsp, *; mov r13, rcx; movaps [rsp+*], xmm7; mov rcx, [rcx+*]; movaps xmm7, xmm1")
        .Get();

    static auto ApplyDeathDurabilityPenalty = Anchor::VirtualFunction<void (*)(AFortPlayerControllerZone*, float)>(TEXT("AFortPlayerControllerZone::OnPawnRevived"))
        .Event(TEXT("ClientOnPawnRevived"))
        .Log(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Or()
        .Event(TEXT("ClientOnPawnRevived"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Get(sizeof(void*));

    if (bApplyDeathDurabilityPenalty)
        ApplyDeathDurabilityPenalty(this, DurabilityPenalty);
}

FFortPlayerDeathReport& AFortPlayerControllerZone::GetPlayerDeathReport()
{
    static auto PlayerDeathReport = GetClass()->FindPropertyByName(FName(TEXT("PlayerDeathReport")));

    return *PlayerDeathReport->ContainerPtrToValuePtr<FFortPlayerDeathReport>(this);
}
