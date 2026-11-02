#include "pch.h"
#include "Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortAthenaAircraft.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Items/FortQuickBars.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Templates/Casts.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
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
    if (!FParse::Param(FCommandLine::Get(), TEXT("nomcp")))
    {
        GetWarmupCountdownDuration() = 60.0f;
    }
    else
    {
        GetWarmupCountdownDuration() = 15.0f;
        GetWarmupEarlyCountdownDuration() = 15.0f;
    }

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
    else
        SpawnAircrafts();

    SetGamePhase(EAthenaGamePhase::Warmup);

    return true;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateWarmupPhase()
{
    static auto OnWarmupCountdownEndTimeUpdated = AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("OnWarmupCountdownEndTimeUpdated")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto GameMode = GameState ? Cast<AFortGameModeAthena>(GameState->GetWorld()->GetAuthGameMode()) : nullptr;

    if (!GameMode)
        return;

    const float ServerWorldTimeSeconds = GameState->GetServerWorldTimeSeconds();
    auto& WarmupCountdownEndTime = GetWarmupCountdownEndTime();

    if (WarmupCountdownEndTime < 0.0f)
    {
        GetWarmupCountdownStartTime() = ServerWorldTimeSeconds;
        WarmupCountdownEndTime = ServerWorldTimeSeconds + GetWarmupCountdownDuration();
        return;
    }

    if (ServerWorldTimeSeconds >= WarmupCountdownEndTime)
    {
        if (ServerWorldTimeSeconds > WarmupCountdownEndTime)
            TryToProceedToAircraftPhase();

        return;
    }

    if (WarmupCountdownEndTime - ServerWorldTimeSeconds <= GetWarmupEarlyCountdownDuration())
        return;

    if (GameMode->CountReadyPlayers() < FMath::CeilToInt(GameMode->GetMaxPlayerCount() * GetWarmupEarlyRequiredPlayerPercent()))
        return;

    float NewWarmupCountdownEndTime = ServerWorldTimeSeconds + GetWarmupEarlyCountdownDuration();
    WarmupCountdownEndTime = NewWarmupCountdownEndTime;

    if (OnWarmupCountdownEndTimeUpdated)
        OnWarmupCountdownEndTimeUpdated->ContainerPtrToValuePtr<FMulticastScriptDelegate>(GameState)->ProcessMulticastDelegate<UObject>(&NewWarmupCountdownEndTime);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SpawnAircrafts()
{
    static auto AirCraftBehavior = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("AirCraftBehavior")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    if (!Playlist)
        return;

    int32 AircraftCount = 1;

    switch (*AirCraftBehavior->ContainerPtrToValuePtr<EAirCraftBehavior>(Playlist))
    {
    case EAirCraftBehavior::OpposingAirCraftForEachTeam:
        AircraftCount = GameState->GetTeamCount();
        break;
    case EAirCraftBehavior::NoAircraft:
        return;
    }

    for (int32 Index = 0; Index < AircraftCount; Index++)
        GetAircrafts_GameMode().Add(AFortAthenaAircraft::SpawnAircraft(GameState->GetWorld(), Index));
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::TryToProceedToAircraftPhase()
{
    if (!StartAircraftPhase(false))
        SetGamePhase(EAthenaGamePhase::None);
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartAircraftPhase(bool bSkippedWarmup)
{
    static auto PlaylistClass = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena"));
    static auto bSkipAircraft = (UBoolProperty*)PlaylistClass->FindPropertyByName(FName(TEXT("bSkipAircraft")));
    static auto AirCraftBehavior = PlaylistClass->FindPropertyByName(FName(TEXT("AirCraftBehavior")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    if (!GameState || (Playlist && bSkipAircraft->GetPropertyValue_InContainer(Playlist)) || GetGamePhase() > EAthenaGamePhase::Aircraft)
        return false;

    if (GetGamePhase() == EAthenaGamePhase::Aircraft)
        return true;

    if (!Playlist)
        return false;

    auto World = GameState->GetWorld();
    auto Behavior = *AirCraftBehavior->ContainerPtrToValuePtr<EAirCraftBehavior>(Playlist);

    if (!bSkippedWarmup && Behavior != EAirCraftBehavior::NoAircraft)
    {
        const int32 AircraftCount = Behavior == EAirCraftBehavior::OpposingAirCraftForEachTeam ? GameState->GetTeamCount() : 1;
        const float ServerWorldTimeSeconds = GameState->GetServerWorldTimeSeconds();

        for (int32 Index = 0; Index < AircraftCount; Index++)
        {
            auto Aircraft = Index < GetAircrafts_GameMode().Num() ? GetAircrafts_GameMode()[Index].Get() : nullptr;

            if (!Aircraft)
                Aircraft = AFortAthenaAircraft::SpawnAircraft(World, Index);

            if (!Aircraft)
                return false;

            SetupAircraft(Aircraft);
            Aircraft->StartFlightPath(ServerWorldTimeSeconds);

            FScriptDelegate OnAircraftEnteredDropZone;
            OnAircraftEnteredDropZone.BindUFunction(this, FName(TEXT("OnAircraftEnteredDropZone")));
            Aircraft->GetOnDropZoneEntered().AddUnique(OnAircraftEnteredDropZone);

            FScriptDelegate OnAircraftExitedDropZone;
            OnAircraftExitedDropZone.BindUFunction(this, FName(TEXT("OnAircraftExitedDropZone")));
            Aircraft->GetOnDropZoneExited().AddUnique(OnAircraftExitedDropZone);

            FScriptDelegate OnAircraftFlightEnded;
            OnAircraftFlightEnded.BindUFunction(this, FName(TEXT("OnAircraftFlightEnded")));
            Aircraft->GetOnFlightEnded().AddUnique(OnAircraftFlightEnded);
        }

        GetAircraftStartTime() = ServerWorldTimeSeconds;
        GetAircraftRealStartTime() = GameState->GetServerWorldRealTimeSeconds();
        GameState->StartMatchStartTimer();
    }

    for (auto PlayerState : GameState->GetPlayerArray())
    {
        auto PlayerController = PlayerState ? Cast<AFortPlayerControllerAthena>(PlayerState->GetOwner()) : nullptr;

        if (!PlayerController)
            continue;

        PlayerController->GetLastDamager() = nullptr;
        PlayerController->GetLastFallInstigator() = nullptr;

        auto Pawn = PlayerController->GetPawn();

        if (auto PlayerPawn = Cast<AFortPlayerPawnAthena>(Pawn))
        {
            PlayerPawn->SetIsInAnyStorm(false);
            PlayerPawn->GetOnEnteredAircraft().ProcessMulticastDelegate<UObject>(nullptr);
        }

        PlayerController->ClientActivateSlot(EFortQuickBars::Primary, 0, 0.0f, true, false);

        if (Pawn)
            Pawn->Destroy();

        PlayerController->Reset();
        PlayerController->ClientGotoState(FName(TEXT("Inactive")));
    }

    SetGamePhase(EAthenaGamePhase::Aircraft);

    return true;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetupAircraft(AFortAthenaAircraft* Aircraft)
{
    if (GetAircrafts_GameState().Num() == 0)
        SetAircraftIsLocked(true);

    GetAircrafts_GameState().Add(Aircraft);
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

float& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetWarmupCountdownStartTime()
{
    static auto WarmupCountdownStartTime = StaticClass()->FindPropertyByName(FName(TEXT("WarmupCountdownStartTime")));

    return *WarmupCountdownStartTime->ContainerPtrToValuePtr<float>(this);
}

float& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetWarmupCountdownEndTime()
{
    static auto WarmupCountdownEndTime = StaticClass()->FindPropertyByName(FName(TEXT("WarmupCountdownEndTime")));

    return *WarmupCountdownEndTime->ContainerPtrToValuePtr<float>(this);
}

float& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetWarmupCountdownDuration()
{
    static auto WarmupCountdownDuration = StaticClass()->FindPropertyByName(FName(TEXT("WarmupCountdownDuration")));

    return *WarmupCountdownDuration->ContainerPtrToValuePtr<float>(this);
}

float& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetWarmupEarlyCountdownDuration()
{
    static auto WarmupEarlyCountdownDuration = StaticClass()->FindPropertyByName(FName(TEXT("WarmupEarlyCountdownDuration")));

    return *WarmupEarlyCountdownDuration->ContainerPtrToValuePtr<float>(this);
}

float UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetWarmupEarlyRequiredPlayerPercent() const
{
    static auto PlaylistWarmupEarlyRequiredPlayerPercent = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("WarmupEarlyRequiredPlayerPercent")));
    static auto DefaultWarmupEarlyRequiredPlayerPercent = StaticClass()->FindPropertyByName(FName(TEXT("DefaultWarmupEarlyRequiredPlayerPercent")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    if (Playlist && *PlaylistWarmupEarlyRequiredPlayerPercent->ContainerPtrToValuePtr<float>(Playlist) > 0.0f)
        return *PlaylistWarmupEarlyRequiredPlayerPercent->ContainerPtrToValuePtr<float>(Playlist);

    return FParse::Param(FCommandLine::Get(), TEXT("nomcp")) ? 0.0f : *DefaultWarmupEarlyRequiredPlayerPercent->ContainerPtrToValuePtr<float>(this);
}

TArray<TWeakObjectPtr<AFortAthenaAircraft>>& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircrafts_GameMode()
{
    static auto Aircrafts_GameMode = StaticClass()->FindPropertyByName(FName(TEXT("Aircrafts_GameMode")));

    return *Aircrafts_GameMode->ContainerPtrToValuePtr<TArray<TWeakObjectPtr<AFortAthenaAircraft>>>(this);
}

TArray<TWeakObjectPtr<AFortAthenaAircraft>>& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircrafts_GameState()
{
    static auto Aircrafts_GameState = StaticClass()->FindPropertyByName(FName(TEXT("Aircrafts_GameState")));

    return *Aircrafts_GameState->ContainerPtrToValuePtr<TArray<TWeakObjectPtr<AFortAthenaAircraft>>>(this);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetAircraftIsLocked(bool bInAircraftIsLocked)
{
    static auto bAircraftIsLocked = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bAircraftIsLocked")));

    bAircraftIsLocked->SetPropertyValue_InContainer(this, bInAircraftIsLocked);
}

float& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraftStartTime()
{
    static auto AircraftStartTime = StaticClass()->FindPropertyByName(FName(TEXT("AircraftStartTime")));

    return *AircraftStartTime->ContainerPtrToValuePtr<float>(this);
}

double& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraftRealStartTime()
{
    static auto AircraftRealStartTime = StaticClass()->FindPropertyByName(FName(TEXT("AircraftRealStartTime")));

    return *AircraftRealStartTime->ContainerPtrToValuePtr<double>(this);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetSafeZonesStartTime(float InSafeZonesStartTime)
{
    static auto SafeZonesStartTime = StaticClass()->FindPropertyByName(FName(TEXT("SafeZonesStartTime")));

    *SafeZonesStartTime->ContainerPtrToValuePtr<float>(this) = InSafeZonesStartTime;
}
