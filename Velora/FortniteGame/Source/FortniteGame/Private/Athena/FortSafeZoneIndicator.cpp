#include "pch.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortSafeZoneIndicator::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneIndicator"));

    return Class;
}

void AFortSafeZoneIndicator::OffsetShrinkTimesFromServerTime(float Offset)
{
    DETOUR(AFortSafeZoneIndicator, OffsetShrinkTimesFromServerTime, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Log(TEXT("AFortGameModeAthena::SetSafeZoneIndicatorShrinkTimes"))
        .Find("mov byte ptr [*], 0x0; mov rcx, [*]; call 0x*")
        .Find("call 0x*")
        .IsTarget("ret*")
        .AtCall());

    auto GameState = GetWorld()->GetGameState();

    if (!GameState)
        return;

    const float ShrinkDuration = GetShrinkDuration();

    GetSafeZoneStartShrinkTime() = GameState->GetServerWorldTimeSeconds() + Offset;
    GetSafeZoneFinishShrinkTime() = GetSafeZoneStartShrinkTime() + ShrinkDuration;
}

void AFortSafeZoneIndicator::SetShrinkDuration(float ShrinkDuration)
{
    DETOUR(AFortSafeZoneIndicator, SetShrinkDuration, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Find("mov rcx, [*]; movaps xmm1, *; call 0x*")
        .Find("call 0x*")
        .IsTarget("ret*")
        .AtCall());

    GetSafeZoneFinishShrinkTime() = GetSafeZoneStartShrinkTime() + ShrinkDuration;
}

float AFortSafeZoneIndicator::GetShrinkDuration()
{
    return FMath::Max(GetSafeZoneFinishShrinkTime() - GetSafeZoneStartShrinkTime(), 0.0f);
}

void AFortSafeZoneIndicator::OnSafeZoneTimerPauseChanged(bool bIsPaused)
{
    DETOUR(AFortSafeZoneIndicator, OnSafeZoneTimerPauseChanged, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("PAUSESAFEZONE"))
        .Find("setz dl; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movzx edx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("PAUSESAFEZONE"))
        .Find("movzx edx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .AtStubBranches(), Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("AFortGameStateAthena::StartPlaylistTimer(%f) Set PlaylistSecondsRemainingCount to: %d"))
        .AtStubBranches());

    auto GameState = GetWorld()->GetGameState();

    if (!GameState)
        return;

    auto& TimeRemainingWhenPhasePaused = GetTimeRemainingWhenPhasePaused();

    if (bIsPaused)
    {
        if (TimeRemainingWhenPhasePaused < 0.0f)
            TimeRemainingWhenPhasePaused = GetSafeZoneFinishShrinkTime() - GameState->GetServerWorldTimeSeconds();

        return;
    }

    if (TimeRemainingWhenPhasePaused >= 0.0f)
    {
        const float ShrinkDuration = GetShrinkDuration();

        GetSafeZoneFinishShrinkTime() = GameState->GetServerWorldTimeSeconds() + TimeRemainingWhenPhasePaused;
        GetSafeZoneStartShrinkTime() = GetSafeZoneFinishShrinkTime() - ShrinkDuration;
    }

    TimeRemainingWhenPhasePaused = -1.0f;
}

float& AFortSafeZoneIndicator::GetSafeZoneStartShrinkTime()
{
    static auto SafeZoneStartShrinkTime = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneStartShrinkTime")));

    return *SafeZoneStartShrinkTime->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetSafeZoneFinishShrinkTime()
{
    static auto SafeZoneFinishShrinkTime = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneFinishShrinkTime")));

    return *SafeZoneFinishShrinkTime->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetTimeRemainingWhenPhasePaused()
{
    static auto TimeRemainingWhenPhasePaused = GetClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused")));

    return *TimeRemainingWhenPhasePaused->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetNextNextRadius()
{
    static auto NextNextRadius = GetClass()->FindPropertyByName(FName(TEXT("NextNextRadius")));

    return *NextNextRadius->ContainerPtrToValuePtr<float>(this);
}

FVector& AFortSafeZoneIndicator::GetNextNextCenter()
{
    static auto NextNextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextNextCenter")));

    return *NextNextCenter->ContainerPtrToValuePtr<FVector>(this);
}

AFortSafeZoneIndicatorFuture* AFortSafeZoneIndicator::GetFutureReplicator() const
{
    static auto FutureReplicator = GetClass()->FindPropertyByName(FName(TEXT("FutureReplicator")));

    return FutureReplicator ? *FutureReplicator->ContainerPtrToValuePtr<AFortSafeZoneIndicatorFuture*>(this) : nullptr;
}
