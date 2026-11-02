#include "pch.h"
#include "Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Athena/FortGameStateAthena.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic"));

    return Class;
}

UFortGameStateComponent_BattleRoyaleGamePhaseLogic* UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_BattleRoyaleGamePhaseLogic*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::HandleMatchHasStarted()
{
    StartWarmupPhase();
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartWarmupPhase()
{
    static auto PlaylistClass = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena"));
    static auto bSkipWarmup = (UBoolProperty*)PlaylistClass->FindPropertyByName(FName(TEXT("bSkipWarmup")));
    static auto bSkipAircraft = (UBoolProperty*)PlaylistClass->FindPropertyByName(FName(TEXT("bSkipAircraft")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());

    if (!GameState)
        return false;

    auto Playlist = GameState->GetCurrentPlaylist();

    if (GetGamePhase() == EAthenaGamePhase::Warmup)
        return true;

    if (Playlist && bSkipWarmup->GetPropertyValue_InContainer(Playlist))
        return false;

    if (Playlist && bSkipAircraft->GetPropertyValue_InContainer(Playlist))
        SetGameModeWillSkipAircraft(true);

    SetGamePhase(EAthenaGamePhase::Warmup);

    return true;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase(EAthenaGamePhase InGamePhase)
{
    static auto SetGamePhase = Anchor::Function<void (*)(UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, EAthenaGamePhase)>(TEXT("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase"))
        .String("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase")
        .Start()
        .Get();

    SetGamePhase(this, InGamePhase);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGameModeWillSkipAircraft(bool bInGameModeWillSkipAircraft)
{
    static auto bGameModeWillSkipAircraft = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGameModeWillSkipAircraft")));

    bGameModeWillSkipAircraft->SetPropertyValue_InContainer(this, bInGameModeWillSkipAircraft);
}

EAthenaGamePhase UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetGamePhase() const
{
    static auto GamePhase = StaticClass()->FindPropertyByName(FName(TEXT("GamePhase")));

    return *GamePhase->ContainerPtrToValuePtr<EAthenaGamePhase>(this);
}

EAthenaGamePhaseStep UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetGamePhaseStep() const
{
    static auto GamePhaseStep = StaticClass()->FindPropertyByName(FName(TEXT("GamePhaseStep")));

    return *GamePhaseStep->ContainerPtrToValuePtr<EAthenaGamePhaseStep>(this);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetSafeZonesStartTime(float InSafeZonesStartTime)
{
    static auto SafeZonesStartTime = StaticClass()->FindPropertyByName(FName(TEXT("SafeZonesStartTime")));

    *SafeZonesStartTime->ContainerPtrToValuePtr<float>(this) = InSafeZonesStartTime;
}
