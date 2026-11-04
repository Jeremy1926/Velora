#include "pch.h"
#include "Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortAthenaAircraft.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Components/FortControllerComponent_Aircraft.h"
#include "Items/FortQuickBars.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffectTypes.h"
#include "Templates/Casts.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

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

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::TickComponent(float DeltaTime)
{
    auto GameState = Cast<AFortGameStateAthena>(GetOwner());

    if (!GameState)
        return;

    const EAthenaGamePhase GamePhase = GetGamePhase();
    auto Playlist = GameState->GetCurrentPlaylist();

    if (Playlist && HasSafeZoneAlreadyStarted(GamePhase, GameState->GetSafeZoneStartUp()))
        UpdateSafeZonesPhase();

    if (GamePhase == EAthenaGamePhase::Warmup)
        UpdateWarmupPhase();
    else if (GamePhase == EAthenaGamePhase::SafeZones && !Playlist)
        UpdateSafeZonesPhase();
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

    GetNumAircraftsExitedDropZone() = 0;

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

    if (GameState->GetSafeZoneStartUp() == ESafeZoneStartUp::StartsWithWarmUp)
        StartSafeZones(false, false);

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

    if (GameState->GetSafeZoneStartUp() == ESafeZoneStartUp::StartsWithAirCraft)
        StartSafeZones(false, false);

    SetGamePhase(EAthenaGamePhase::Aircraft);

    return true;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetupAircraft(AFortAthenaAircraft* Aircraft)
{
    if (GetAircrafts_GameState().Num() == 0)
        SetAircraftIsLocked(true);

    GetAircrafts_GameState().Add(Aircraft);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::execOnAircraftEnteredDropZone(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameStateComponent_BattleRoyaleGamePhaseLogic, execOnAircraftEnteredDropZone, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftEnteredDropZone"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftEnteredDropZone")));

    AFortAthenaAircraft* FortAthenaAircraft = nullptr;

    Stack.StepCompiledIn(&FortAthenaAircraft);
    Stack.Code += !!Stack.Code;

    OnAircraftEnteredDropZone(FortAthenaAircraft);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::OnAircraftEnteredDropZone(AFortAthenaAircraft* FortAthenaAircraft)
{
    bool bAllAircraftsInDropZone = true;

    for (int32 AircraftIndex = 0; AircraftIndex < GetAircrafts_GameState().Num(); AircraftIndex++)
    {
        if (auto Aircraft = GetAircraft(AircraftIndex))
            bAllAircraftsInDropZone &= Aircraft->IsInDropZone();
    }

    if (bAllAircraftsInDropZone)
        SetAircraftLock(false);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::execOnAircraftExitedDropZone(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameStateComponent_BattleRoyaleGamePhaseLogic, execOnAircraftExitedDropZone, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftExitedDropZone"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftExitedDropZone")));

    AFortAthenaAircraft* FortAthenaAircraft = nullptr;

    Stack.StepCompiledIn(&FortAthenaAircraft);
    Stack.Code += !!Stack.Code;

    OnAircraftExitedDropZone(FortAthenaAircraft);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::OnAircraftExitedDropZone(AFortAthenaAircraft* FortAthenaAircraft)
{
    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto GameMode = GameState ? GameState->GetWorld()->GetAuthGameMode() : nullptr;

    if (!GameMode)
        return;

    for (auto PlayerState : GameState->GetPlayerArray())
    {
        auto PlayerController = PlayerState ? Cast<AFortPlayerControllerAthena>(PlayerState->GetOwner()) : nullptr;

        if (!PlayerController || GetAircraft(GetAircraftIndex(PlayerState)) != FortAthenaAircraft)
            continue;

        auto AircraftComponent = PlayerController->FindComponentByClass<UFortControllerComponent_Aircraft>();

        if (!AircraftComponent || !AircraftComponent->IsInAircraft())
            continue;

        AircraftComponent->GetIsAttemptingAircraftJump() = true;
        GameMode->RestartPlayer(PlayerController);
        AircraftComponent->GetIsAttemptingAircraftJump() = false;
    }

    if (++GetNumAircraftsExitedDropZone() < GetAircrafts_GameState().Num())
        return;

    const ESafeZoneStartUp SafeZoneStartUp = GameState->GetSafeZoneStartUp();

    if (SafeZoneStartUp == ESafeZoneStartUp::StartsWithWarmUp || SafeZoneStartUp == ESafeZoneStartUp::StartsWithAirCraft)
        StartSafeZonesPhase(false);
    else
        StartSafeZones(false, true);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::execOnAircraftFlightEnded(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameStateComponent_BattleRoyaleGamePhaseLogic, execOnAircraftFlightEnded, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftFlightEnded"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:OnAircraftFlightEnded")));

    AFortAthenaAircraft* FortAthenaAircraft = nullptr;

    Stack.StepCompiledIn(&FortAthenaAircraft);
    Stack.Code += !!Stack.Code;

    OnAircraftFlightEnded(FortAthenaAircraft);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::OnAircraftFlightEnded(AFortAthenaAircraft* FortAthenaAircraft)
{
    if (!FortAthenaAircraft)
        return;

    FScriptDelegate OnAircraftEnteredDropZone;
    OnAircraftEnteredDropZone.BindUFunction(this, FName(TEXT("OnAircraftEnteredDropZone")));
    FortAthenaAircraft->GetOnDropZoneEntered().Remove(OnAircraftEnteredDropZone);

    FScriptDelegate OnAircraftExitedDropZone;
    OnAircraftExitedDropZone.BindUFunction(this, FName(TEXT("OnAircraftExitedDropZone")));
    FortAthenaAircraft->GetOnDropZoneExited().Remove(OnAircraftExitedDropZone);

    FScriptDelegate OnAircraftFlightEnded;
    OnAircraftFlightEnded.BindUFunction(this, FName(TEXT("OnAircraftFlightEnded")));
    FortAthenaAircraft->GetOnFlightEnded().Remove(OnAircraftFlightEnded);

    FortAthenaAircraft->Destroy(false, true);
}

AFortAthenaAircraft* UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraft(int32 AircraftIndex) const
{
    static auto GetAircraft = Anchor::Function<AFortAthenaAircraft* (*)(const UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, int32)>(TEXT("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraft"))
        .String("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraft")
        .Start()
        .Get();

    return GetAircraft(this, AircraftIndex);
}

int32 UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraftIndex(APlayerState* PlayerState)
{
    auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerState);

    return PlayerStateAthena ? GetAircraftIndexFromTeam(PlayerStateAthena->GetTeamIndex()) : 0;
}

int32 UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetAircraftIndexFromTeam(uint8 TeamIndex)
{
    static auto DefaultFirstTeam = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("DefaultFirstTeam")));
    static auto AirCraftBehavior = StaticClass()->FindPropertyByName(FName(TEXT("AirCraftBehavior")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    if (!Playlist || *AirCraftBehavior->ContainerPtrToValuePtr<EAirCraftBehavior>(this) != EAirCraftBehavior::OpposingAirCraftForEachTeam)
        return 0;

    const int32 AircraftIndex = TeamIndex - *DefaultFirstTeam->ContainerPtrToValuePtr<uint8>(Playlist);

    if (AircraftIndex < 0)
        return 0;

    if (GetGamePhase() == EAthenaGamePhase::Setup || GetGamePhase() == EAthenaGamePhase::Warmup || AircraftIndex < GetAircrafts_GameState().Num())
        return AircraftIndex;

    return 0;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetAircraftLock(bool bLock)
{
    static auto SetAircraftLock = Anchor::Function<void (*)(UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, bool)>(TEXT("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetAircraftLock"))
        .FieldPattern("80 B9 %o 00 ? ? ? 48 8B D9 74 ? 84 D2 75 ? 80 B9 ? ? 00 00 03 77", StaticClass()->FindPropertyByName(FName(TEXT("bAircraftIsLocked"))))
        .Start()
        .Get();

    SetAircraftLock(this, bLock);
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::HasSafeZoneAlreadyStarted(EAthenaGamePhase InGamePhase, ESafeZoneStartUp SafeZoneStartUp)
{
    switch (InGamePhase)
    {
    case EAthenaGamePhase::Warmup:
        return SafeZoneStartUp == ESafeZoneStartUp::StartsWithWarmUp;
    case EAthenaGamePhase::Aircraft:
        return SafeZoneStartUp == ESafeZoneStartUp::StartsWithWarmUp || SafeZoneStartUp == ESafeZoneStartUp::StartsWithAirCraft;
    case EAthenaGamePhase::SafeZones:
        return true;
    default:
        return false;
    }
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartSafeZones(bool bForce, bool bStartSafeZonesPhase)
{
    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;

    if (!MapInfo || MapInfo->GetSafeZoneCount() < 2)
        return false;

    if (!bForce && (GetGamePhase() == EAthenaGamePhase::SafeZones || (GameState->GetCurrentPlaylist() && HasSafeZoneAlreadyStarted(GetGamePhase(), GameState->GetSafeZoneStartUp()))))
        return true;

    GameState->SetSafeZonesStartTime(MapInfo->GetSafeZoneStartDelay().GetValueAtLevel(0.0f) + GameState->GetServerWorldTimeSeconds());

    return bStartSafeZonesPhase ? StartSafeZonesPhase(bForce) : true;
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StartSafeZonesPhase(bool bForce)
{
    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;

    if (!MapInfo || MapInfo->GetSafeZoneCount() < 2)
        return false;

    if (GetGamePhase() != EAthenaGamePhase::SafeZones || bForce)
        SetGamePhase(EAthenaGamePhase::SafeZones);

    return true;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SpawnInitialSafeZone()
{
    static auto SafeZoneIndicatorClass = StaticClass()->FindPropertyByName(FName(TEXT("SafeZoneIndicatorClass")));
    static auto OnSafeZoneIndicatorSpawned = AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("OnSafeZoneIndicatorSpawned")));

    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());
    auto& SafeZoneIndicator = GetSafeZoneIndicator();

    if (!GameMode)
        return;

    auto Class = *SafeZoneIndicatorClass->ContainerPtrToValuePtr<UClass*>(this);

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    SafeZoneIndicator = (AFortSafeZoneIndicator*)GetWorld()->SpawnActor(Class && Class->IsChildOf(AFortSafeZoneIndicator::StaticClass()) ? Class : nullptr, nullptr, nullptr, SpawnParameters);

    if (!SafeZoneIndicator)
        return;

    if (OnSafeZoneIndicatorSpawned)
        OnSafeZoneIndicatorSpawned->ContainerPtrToValuePtr<FMulticastScriptDelegate>(GameMode)->ProcessMulticastDelegate<UObject>(&SafeZoneIndicator);

    SafeZoneIndicator->InitializeWithMapDefaultPhases();
    GameMode->SetSafeZonePhase(-1);
    GameMode->StartNewSafeZonePhase(-1);
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::UpdateSafeZonesPhase()
{
    static auto bSafezoneEventDriven = (UBoolProperty*)AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("bSafezoneEventDriven")));
    static auto bPausedForPreview = (UBoolProperty*)AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("bPausedForPreview")));
    static auto PhaseCount = AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("PhaseCount")));

    auto GameState = Cast<AFortGameStateAthena>(GetOwner());
    auto GameMode = GameState ? Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()) : nullptr;

    if (!GameMode)
        return;

    const float ServerWorldTimeSeconds = GameState->GetServerWorldTimeSeconds();

    if (!GetSafeZoneIndicator() && ServerWorldTimeSeconds > GetSafeZonesStartTime())
        SpawnInitialSafeZone();

    const int32 SafeZonePhase = GameMode->GetSafeZonePhase();
    auto SafeZoneIndicator = GetSafeZoneIndicator();

    if (IsSafeZoneTimerPaused() || !SafeZoneIndicator || GetPlaylistStoppedSafeZonePhases())
        return;

    const bool bStartNewSafeZonePhase = ServerWorldTimeSeconds >= SafeZoneIndicator->GetSafeZoneFinishShrinkTime() && !bSafezoneEventDriven->GetPropertyValue_InContainer(SafeZoneIndicator);

    if (GetGamePhase() == EAthenaGamePhase::Warmup || GetGamePhase() == EAthenaGamePhase::Aircraft)
    {
        if (!bPausedForPreview->GetPropertyValue_InContainer(SafeZoneIndicator) && !SafeZonePhase && bStartNewSafeZonePhase)
            GameMode->StartNewSafeZonePhase(-1);

        bPausedForPreview->SetPropertyValue_InContainer(SafeZoneIndicator, true);
        return;
    }

    bPausedForPreview->SetPropertyValue_InContainer(SafeZoneIndicator, false);

    if (bStartNewSafeZonePhase && SafeZonePhase < *PhaseCount->ContainerPtrToValuePtr<int32>(SafeZoneIndicator) - 1)
        GameMode->StartNewSafeZonePhase(-1);
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::IsSafeZoneTimerPaused() const
{
    static auto SafeZonePauseTime = StaticClass()->FindPropertyByName(FName(TEXT("SafeZonePauseTime")));

    return *SafeZonePauseTime->ContainerPtrToValuePtr<float>(this) >= 0.0f;
}

void UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase(EAthenaGamePhase InGamePhase)
{
    static auto SetGamePhase = Anchor::Function<void (*)(UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, EAthenaGamePhase)>(TEXT("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase"))
        .String("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SetGamePhase")
        .Start()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic:SetGamePhase"))
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

int32& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetNumAircraftsExitedDropZone()
{
    static int32 NumAircraftsExitedDropZone = 0;

    return NumAircraftsExitedDropZone;
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

float UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetSafeZonesStartTime() const
{
    static auto SafeZonesStartTime = StaticClass()->FindPropertyByName(FName(TEXT("SafeZonesStartTime")));

    return *SafeZonesStartTime->ContainerPtrToValuePtr<float>(this);
}

AFortSafeZoneIndicator*& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetSafeZoneIndicator()
{
    static auto SafeZoneIndicator = StaticClass()->FindPropertyByName(FName(TEXT("SafeZoneIndicator")));

    return *SafeZoneIndicator->ContainerPtrToValuePtr<AFortSafeZoneIndicator*>(this);
}

TArray<FVector>& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetSafeZoneLocations()
{
    static auto SafeZoneLocations = Anchor::Offset(TEXT("UFortGameStateComponent_BattleRoyaleGamePhaseLogic::SafeZoneLocations"))
        .String(TEXT("InitializeFlightPath: CustomDropZoneBox2D is invalid when bUseCustomDropZone is true"))
        .Start()
        .Find("cmp *, [*+0x*]; jnl 0x*; mov rax, [*+0x*]")
        .Find("mov rax, [*+0x*]")
        .Or()
        .String(TEXT("InitializeFlightPath: CustomDropZoneBox2D is invalid when bUseCustomDropZone is true"))
        .Start()
        .Find("cmp *, [rax]; jnl 0x*; mov rax, [*+0x*]")
        .Find("mov rax, [*+0x*]")
        .Disp();

    return *(TArray<FVector>*)((uint8*)this + SafeZoneLocations);
}

bool UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetPlaylistStoppedSafeZonePhases() const
{
    static auto bPlaylistStoppedSafeZonePhases = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistStoppedSafeZonePhases")));

    return bPlaylistStoppedSafeZonePhases->GetPropertyValue_InContainer(this);
}

const FScalableFloat& UFortGameStateComponent_BattleRoyaleGamePhaseLogic::GetSafeZoneDamageAttribute() const
{
    static auto SafeZoneDamageAttribute = StaticClass()->FindPropertyByName(FName(TEXT("SafeZoneDamageAttribute")));

    return *SafeZoneDamageAttribute->ContainerPtrToValuePtr<FScalableFloat>(this);
}
