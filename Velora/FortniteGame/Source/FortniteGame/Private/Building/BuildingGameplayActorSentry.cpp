#include "pch.h"
#include "Building/BuildingGameplayActorSentry.h"
#include "GameplayEffectTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SceneComponent.h"
#include "Containers/Map.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct FPendingSentryAggro
{
    TWeakObjectPtr<ABuildingGameplayActorSentry> Sentry;
    float Time;
};

static TArray<FPendingSentryAggro> PendingAggros;

static TMap<const ABuildingGameplayActorSentry*, FTimerHandle> TrackedToAggroTimerHandles;

UClass* ABuildingGameplayActorSentry::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorSentry"));

    return Class;
}

void ABuildingGameplayActorSentry::SetSentryState(EBuildingGameplayActorSentry_State InSentryState)
{
    const auto PreviousSentryState = GetCurrentSentryState();
    auto ActorBeingTracked = GetActorBeingTracked();

    FlushNetDormancy();

    if (auto ReturnToIdleTimerHandle = GetReturnToIdleTimerHandle())
        UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, *ReturnToIdleTimerHandle);

    GetCurrentSentryState() = InSentryState;

    UE_LOG(LogVelora, Log, TEXT("[Sentry] %s state %d -> %d, tracking %s"), *GetName(), (int32)PreviousSentryState, (int32)InSentryState, ActorBeingTracked ? *ActorBeingTracked->GetName() : TEXT("none"));

    OnRep_CurrentSentryState(PreviousSentryState);
}

void ABuildingGameplayActorSentry::execSetSentryState(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingGameplayActorSentry, execSetSentryState, Anchor::Hook()
        .StubExec(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:SetSentryState"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:SetSentryState")));

    EBuildingGameplayActorSentry_State InSentryState{};

    Stack.StepCompiledIn(&InSentryState);
    Stack.Code += !!Stack.Code;

    SetSentryState(InSentryState);
}

void ABuildingGameplayActorSentry::SetActorToTrack(AActor* InActorToTrack)
{
    if (!HasAuthority())
        return;

    FlushNetDormancy();

    const auto TrackedActorPrevious = GetActorBeingTracked();

    GetActorBeingTracked() = InActorToTrack;
    BPOnTrackedActorSet(TrackedActorPrevious);

    auto World = GetWorld();

    if (!InActorToTrack || GetCurrentSentryState() == EBuildingGameplayActorSentry_State::Aggro || !World)
        return;

    const float TrackedToAggroDelay = GetTrackedToAggroDelay().GetValueAtLevel(0.0f) * FMath::Max(GetTrackedToAggroDelayMultiplier(), 0.0f);

    if (FTimerManager::CanSetTimers())
    {
        World->GetTimerManager().SetTimer(TrackedToAggroTimerHandles.FindOrAdd(this), this, &ABuildingGameplayActorSentry::StartAggro, TrackedToAggroDelay);
    }
    else
    {
        for (int32 Index = PendingAggros.Num() - 1; Index >= 0; Index--)
        {
            if (PendingAggros[Index].Sentry.Get() == this)
                PendingAggros.RemoveAt(Index);
        }

        PendingAggros.Add(FPendingSentryAggro{ this, (float)World->GetTimeSeconds() + TrackedToAggroDelay });
    }

    SetSentryState(EBuildingGameplayActorSentry_State::Tracking);
    K2_ExecuteGameplayCue(GetTrackedGameplayCueTag(), FGameplayEffectContextHandle());
}

void ABuildingGameplayActorSentry::execSetActorToTrack(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingGameplayActorSentry, execSetActorToTrack, Anchor::Hook()
        .StubExec(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:SetActorToTrack"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:SetActorToTrack")));

    AActor* InActorToTrack = nullptr;

    Stack.StepCompiledIn(&InActorToTrack);
    Stack.Code += !!Stack.Code;

    SetActorToTrack(InActorToTrack);
}

void ABuildingGameplayActorSentry::BPOnTrackedActorSet(AActor* TrackedActorPrevious)
{
    static auto BPOnTrackedActorSet = Anchor::Function<void (*)(ABuildingGameplayActorSentry*, AActor*)>(TEXT("ABuildingGameplayActorSentry::BPOnTrackedActorSet"))
        .Event(TEXT("BPOnTrackedActorSet"))
        .Optional()
        .Get();

    auto ActorBeingTracked = GetActorBeingTracked();

    UE_LOG(LogVelora, Log, TEXT("[Sentry] %s BPOnTrackedActorSet %s -> %s through %s"), *GetName(), TrackedActorPrevious ? *TrackedActorPrevious->GetName() : TEXT("none"), ActorBeingTracked ? *ActorBeingTracked->GetName() : TEXT("none"), BPOnTrackedActorSet ? TEXT("the event thunk") : TEXT("ProcessEvent"));

    if (BPOnTrackedActorSet)
        return BPOnTrackedActorSet(this, TrackedActorPrevious);

    auto Function = FindFunctionChecked(FName(TEXT("BPOnTrackedActorSet")));
    auto Parameter = Function ? Function->GetPropertyLink() : nullptr;

    if (!Parameter)
        return;

    TArray<uint8> Parms;
    Parms.SetNum(Function->GetPropertiesSize());
    memset(Parms.GetData(), 0, Parms.Num());

    *Parameter->ContainerPtrToValuePtr<AActor*>(Parms.GetData()) = TrackedActorPrevious;

    ProcessEvent(Function, Parms.GetData());
}

void ABuildingGameplayActorSentry::StartAggro()
{
    if (!GetActorBeingTracked())
        return;

    SetSentryState(EBuildingGameplayActorSentry_State::Aggro);
    K2_ExecuteGameplayCue(GetAggroedGameplayCueTag(), FGameplayEffectContextHandle());
}

void ABuildingGameplayActorSentry::TickPendingAggros()
{
    for (int32 Index = PendingAggros.Num() - 1; Index >= 0; Index--)
    {
        auto Sentry = PendingAggros[Index].Sentry.Get();

        if (Sentry && Sentry->GetWorld() && Sentry->GetWorld()->GetTimeSeconds() < PendingAggros[Index].Time)
            continue;

        PendingAggros.RemoveAt(Index);

        if (Sentry)
            Sentry->StartAggro();
    }
}

void ABuildingGameplayActorSentry::HandleDamaged(float Damage, const FGameplayTagContainer& DamageTags, const TGameValue<FVector>& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext)
{
    DETOUR(ABuildingGameplayActorSentry, HandleDamaged, Anchor::Hook()
        .Event(TEXT("OnDamageServer"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__BuildingGameplayActor"))
        .VirtualIs(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "jmp %t")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry")));

    Original(Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

    const auto CurrentSentryState = GetCurrentSentryState();

    if (!GetbLocateDamagerIfHitWhileIdle() || !DamageCauser || !HasAuthority() || (CurrentSentryState > EBuildingGameplayActorSentry_State::ActiveIdle && CurrentSentryState != EBuildingGameplayActorSentry_State::ReturningToIdle))
        return;

    SetSentryState(EBuildingGameplayActorSentry_State::LocatingDamager);
    SetLocatingDamageRotationDestination((DamageCauser->GetActorLocation() - GetCameraStaticMeshComp()->GetComponentLocation()).GetSafeNormal().ToOrientationRotator());
}

void ABuildingGameplayActorSentry::SetLocatingDamageRotationDestination(const FRotator& InLocatingDamageRotationDestination)
{
    static auto SetLocatingDamageRotationDestination = Anchor::Function<void (*)(ABuildingGameplayActorSentry*, const TGameValue<FRotator>&)>(TEXT("ABuildingGameplayActorSentry::SetLocatingDamageRotationDestination"))
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x7; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .Find("movsd [rbp-0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x7; jnz 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .Find("movsd [rbp-0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("cmp *, 0x7; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .Find("movsd [rbp-0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (SetLocatingDamageRotationDestination)
        SetLocatingDamageRotationDestination(this, TGameValue<FRotator>(InLocatingDamageRotationDestination));
}

void ABuildingGameplayActorSentry::ReturnToIdle()
{
    DETOUR(ABuildingGameplayActorSentry, ReturnToIdle, Anchor::Hook()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x6; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x6; jz 0x*")
        .Find("jz 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("cmp *, 0x6; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Optional()
        .AtReference(), Anchor::Hook()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x7; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x7; jnz 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("cmp *, 0x7; jnz 0x*")
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LocatingDamage_UpdateTarget_PauseTime"))))
        .Find("jbe 0x*")
        .Target()
        .Call()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("PauseTimeBeforeIdleState"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Optional()
        .AtReference());

    const auto CurrentSentryState = GetCurrentSentryState();

    if (CurrentSentryState != EBuildingGameplayActorSentry_State::ReturningToIdle && CurrentSentryState != EBuildingGameplayActorSentry_State::LocatingDamager)
        return;

    if (!GetbDoesNotRotate())
        SelectClosestIdleRotation();

    SetSentryState(GetTargetActorsInRange().Num() > 0 ? EBuildingGameplayActorSentry_State::ActiveIdle : GetbGoDormantAfterPassiveIdle() ? EBuildingGameplayActorSentry_State::Dormant : EBuildingGameplayActorSentry_State::PassiveIdle);
    K2_ExecuteGameplayCue(GetReturnToIdleGameplayCueTag(), FGameplayEffectContextHandle());
}

void ABuildingGameplayActorSentry::IdleRestart()
{
    DETOUR(ABuildingGameplayActorSentry, IdleRestart, Anchor::Hook()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("cmp byte ptr [*+%x], 0x0; jnz 0x*", StaticClass()->FindPropertyByName(FName(TEXT("bDoesNotRotate"))))
        .Find("jnz 0x*", 2)
        .Target()
        .Call()
        .FieldFind("cmp *[*+%x], *; jnz 0x*", StaticClass()->FindPropertyByName(FName(TEXT("bIdleRotationStopped"))))
        .Find("jnz 0x*")
        .Target()
        .FieldFind("movss *, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("IdleRotationPauseTime"))))
        .Find("lea rax, [0x*]")
        .IsTarget("ret*")
        .Optional()
        .AtReference());

    SetbIdleRotationStopped(false);

    const auto FixedRotationGoal = GetbIdleRotatingRight() ? GetTargetRotationRight() : GetTargetRotationLeft();

    SetFixedRotationGoal(FixedRotationGoal);
    SetFixedRotationGoalForReplication(FixedRotationGoal);
}

void ABuildingGameplayActorSentry::SelectClosestIdleRotation()
{
    const auto TargetDirection = GetTargetRotation().Vector();
    const auto TargetRotationLeft = GetTargetRotationLeft();
    const auto TargetRotationRight = GetTargetRotationRight();
    const auto FixedRotationGoal = (TargetDirection | TargetRotationLeft.Vector()) > (TargetDirection | TargetRotationRight.Vector()) ? TargetRotationLeft : TargetRotationRight;

    SetFixedRotationGoal(FixedRotationGoal);
    SetFixedRotationGoalForReplication(FixedRotationGoal);
}

void ABuildingGameplayActorSentry::SetFixedRotationGoal(const FRotator& InFixedRotationGoal)
{
    static auto FixedRotationGoal = Anchor::Offset(TEXT("ABuildingGameplayActorSentry::FixedRotationGoal"))
        .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:OnRep_FixedRotationGoalReplicated"))
        .Find("movups [*+0x*], xmm0")
        .Disp();

    if (FixedRotationGoal)
        TGameLayout<FRotator>::Store((uint8*)this + FixedRotationGoal, InFixedRotationGoal);
}

void ABuildingGameplayActorSentry::SetFixedRotationGoalForReplication(const FRotator& InFixedRotationGoal)
{
    static auto FixedRotationGoalReplicated = StaticClass()->FindPropertyByName(FName(TEXT("FixedRotationGoalReplicated")));

    FlushNetDormancy();
    TGameLayout<FRotator>::Store(FixedRotationGoalReplicated->ContainerPtrToValuePtr<void>(this), InFixedRotationGoal);
}

void ABuildingGameplayActorSentry::OnRep_CurrentSentryState(EBuildingGameplayActorSentry_State PreviousSentryState)
{
    static auto OnRep_CurrentSentryState = Anchor::Function<void (*)(ABuildingGameplayActorSentry*, EBuildingGameplayActorSentry_State)>(TEXT("ABuildingGameplayActorSentry::OnRep_CurrentSentryState"))
        .Native(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:OnRep_CurrentSentryState"))
        .Get();

    OnRep_CurrentSentryState(this, PreviousSentryState);
}

FTimerHandle* ABuildingGameplayActorSentry::GetReturnToIdleTimerHandle()
{
    static auto ReturnToIdleTimerHandle = Anchor::Offset(TEXT("ABuildingGameplayActorSentry::ReturnToIdleTimerHandle"))
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x6; jnz 0x*")
        .Call()
        .Find("lea *, [*+0x*]; mov *, [*]; mov [rsp+0x*], *")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jnb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("jnb 0x*")
        .Target()
        .Find("cmp *, 0x6; jz 0x*")
        .Find("jz 0x*")
        .Target()
        .Call()
        .Find("lea *, [*+0x*]; mov *, [*]; mov [rsp+0x*], *")
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSentry"), "movss xmm*, dword ptr [*+%x]", StaticClass()->FindPropertyByName(FName(TEXT("LastNetworkReceiveTime"))))
        .FieldFind("mov *, [*+%x]; cmp *, 0x2; jb 0x*", StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState"))))
        .Find("cmp *, 0x6; jnz 0x*")
        .Call()
        .Find("lea *, [*+0x*]; mov *, [*]; mov [rsp+0x*], *")
        .Disp();

    return ReturnToIdleTimerHandle ? (FTimerHandle*)((uint8*)this + ReturnToIdleTimerHandle) : nullptr;
}

EBuildingGameplayActorSentry_State& ABuildingGameplayActorSentry::GetCurrentSentryState()
{
    static auto CurrentSentryState = StaticClass()->FindPropertyByName(FName(TEXT("CurrentSentryState")));

    return *CurrentSentryState->ContainerPtrToValuePtr<EBuildingGameplayActorSentry_State>(this);
}

AActor*& ABuildingGameplayActorSentry::GetActorBeingTracked()
{
    static auto ActorBeingTracked = StaticClass()->FindPropertyByName(FName(TEXT("ActorBeingTracked")));

    return *ActorBeingTracked->ContainerPtrToValuePtr<AActor*>(this);
}

FScalableFloat& ABuildingGameplayActorSentry::GetTrackedToAggroDelay()
{
    static auto TrackedToAggroDelay = StaticClass()->FindPropertyByName(FName(TEXT("TrackedToAggroDelay")));

    return *TrackedToAggroDelay->ContainerPtrToValuePtr<FScalableFloat>(this);
}

float ABuildingGameplayActorSentry::GetTrackedToAggroDelayMultiplier()
{
    static auto TrackedToAggroDelayMultiplier = StaticClass()->FindPropertyByName(FName(TEXT("TrackedToAggroDelayMultiplier")));

    return TrackedToAggroDelayMultiplier ? *TrackedToAggroDelayMultiplier->ContainerPtrToValuePtr<float>(this) : 1.0f;
}

FGameplayTag ABuildingGameplayActorSentry::GetTrackedGameplayCueTag()
{
    static auto TrackedGameplayCueTag = StaticClass()->FindPropertyByName(FName(TEXT("TrackedGameplayCueTag")));

    return *TrackedGameplayCueTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}

FGameplayTag ABuildingGameplayActorSentry::GetAggroedGameplayCueTag()
{
    static auto AggroedGameplayCueTag = StaticClass()->FindPropertyByName(FName(TEXT("AggroedGameplayCueTag")));

    return *AggroedGameplayCueTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}

USceneComponent* ABuildingGameplayActorSentry::GetCameraStaticMeshComp()
{
    static auto CameraStaticMeshComp = StaticClass()->FindPropertyByName(FName(TEXT("CameraStaticMeshComp")));

    return *CameraStaticMeshComp->ContainerPtrToValuePtr<USceneComponent*>(this);
}

bool ABuildingGameplayActorSentry::GetbDoesNotRotate()
{
    static auto bDoesNotRotate = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bDoesNotRotate")));

    return bDoesNotRotate->GetPropertyValue_InContainer(this);
}

bool ABuildingGameplayActorSentry::GetbLocateDamagerIfHitWhileIdle()
{
    static auto bLocateDamagerIfHitWhileIdle = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bLocateDamagerIfHitWhileIdle")));

    return bLocateDamagerIfHitWhileIdle->GetPropertyValue_InContainer(this);
}

bool ABuildingGameplayActorSentry::GetbGoDormantAfterPassiveIdle()
{
    static auto bGoDormantAfterPassiveIdle = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGoDormantAfterPassiveIdle")));

    return bGoDormantAfterPassiveIdle->GetPropertyValue_InContainer(this);
}

bool ABuildingGameplayActorSentry::GetbIdleRotatingRight()
{
    static auto bIdleRotatingRight = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bIdleRotatingRight")));

    return bIdleRotatingRight->GetPropertyValue_InContainer(this);
}

void ABuildingGameplayActorSentry::SetbIdleRotationStopped(bool bInIdleRotationStopped)
{
    static auto bIdleRotationStopped = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bIdleRotationStopped")));

    bIdleRotationStopped->SetPropertyValue_InContainer(this, bInIdleRotationStopped);
}

FRotator ABuildingGameplayActorSentry::GetTargetRotation()
{
    static auto TargetRotation = StaticClass()->FindPropertyByName(FName(TEXT("TargetRotation")));

    return TGameLayout<FRotator>::Load(TargetRotation->ContainerPtrToValuePtr<void>(this));
}

FRotator ABuildingGameplayActorSentry::GetTargetRotationRight()
{
    static auto TargetRotationRight = StaticClass()->FindPropertyByName(FName(TEXT("TargetRotationRight")));

    return TGameLayout<FRotator>::Load(TargetRotationRight->ContainerPtrToValuePtr<void>(this));
}

FRotator ABuildingGameplayActorSentry::GetTargetRotationLeft()
{
    static auto TargetRotationLeft = StaticClass()->FindPropertyByName(FName(TEXT("TargetRotationLeft")));

    return TGameLayout<FRotator>::Load(TargetRotationLeft->ContainerPtrToValuePtr<void>(this));
}

TArray<AActor*>& ABuildingGameplayActorSentry::GetTargetActorsInRange()
{
    static auto TargetActorsInRange = StaticClass()->FindPropertyByName(FName(TEXT("TargetActorsInRange")));

    return *TargetActorsInRange->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

FGameplayTag ABuildingGameplayActorSentry::GetReturnToIdleGameplayCueTag()
{
    static auto ReturnToIdleGameplayCueTag = StaticClass()->FindPropertyByName(FName(TEXT("ReturnToIdleGameplayCueTag")));

    return *ReturnToIdleGameplayCueTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}
