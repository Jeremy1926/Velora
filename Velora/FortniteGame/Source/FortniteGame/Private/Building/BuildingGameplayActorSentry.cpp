#include "pch.h"
#include "Building/BuildingGameplayActorSentry.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
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
    GetActorBeingTracked() = InActorToTrack;

    auto World = GetWorld();

    if (!InActorToTrack || GetCurrentSentryState() == EBuildingGameplayActorSentry_State::Aggro || !World)
        return;

    const float TrackedToAggroDelay = GetTrackedToAggroDelay().GetValueAtLevel(0.0f) * FMath::Max(GetTrackedToAggroDelayMultiplier(), 0.0f);

    for (int32 Index = PendingAggros.Num() - 1; Index >= 0; Index--)
    {
        if (PendingAggros[Index].Sentry.Get() == this)
            PendingAggros.RemoveAt(Index);
    }

    PendingAggros.Add(FPendingSentryAggro{ this, (float)World->GetTimeSeconds() + TrackedToAggroDelay });

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

void ABuildingGameplayActorSentry::OnRep_CurrentSentryState(EBuildingGameplayActorSentry_State PreviousSentryState)
{
    static auto OnRep_CurrentSentryState = Anchor::Function<void (*)(ABuildingGameplayActorSentry*, EBuildingGameplayActorSentry_State)>(TEXT("ABuildingGameplayActorSentry::OnRep_CurrentSentryState"))
        .Native(TEXT("/Script/FortniteGame.BuildingGameplayActorSentry:OnRep_CurrentSentryState"))
        .Get();

    OnRep_CurrentSentryState(this, PreviousSentryState);
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
