#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "Athena/Abilities/FortGameplayAbilityAthena_PeriodicItemGrant.h"
#include "Athena/ControllerComponents/FortControllerComponent_TransientQuests.h"
#include "Building/BGAConsumableSpawner.h"
#include "Building/BuildingGameplayActorTransientQuest.h"
#include "Building/BuildingItemCollectorActor.h"
#include "Building/BuildingGameplayActorSpawnMachine.h"
#include "FortAthenaLivingWorldManager.h"
#include "Online/FortGameSession.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "ContentStreaming.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "HAL/PlatformProperties.h"
#include "Misc/EngineVersion.h"
#include "Misc/CommandLine.h"
#include "Online/FortPlaylistManager.h"
#include "FortGameInstance.h"
#include "FortGlobals.h"
#include "Analytics/FortAnalytics.h"
#include "CoreGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    DETOUR(UGameEngine, Tick, Anchor::Hook()
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Inline());

    const TCHAR* State = TEXT("Loading");

    if (!GIsServer)
    {
        auto GameInstance = GetGameInstance();

        GIsClient = false;
        GIsServer = true;

        extern bool& GIsAllowingParticles;
        GIsAllowingParticles = false;

        IStreamingManager::Get().RemoveStreamingViews(RemoveStreamingViews_All);
        IStreamingManager::Get().Tick(0.0f);

        GameInstance->GetWorldContext()->SetRunAsDedicated(true);

        auto FortGameInstance = static_cast<UFortGameInstance*>(GameInstance);
        FortGameInstance->GetServerAnalyticsProvider() = UFortAnalytics::CreateAnalyticsProvider(FString(), FString(), FString(FPlatformProperties::IniPlatformName()), EAnalyticsProviderType::ServerGameInstance, FString());
        FortGameInstance->GetServerAnalyticsProvider()->StartSession();

        if (auto ServerEsportsAnalyticsProvider = FortGameInstance->GetServerEsportsAnalyticsProvider())
        {
            *ServerEsportsAnalyticsProvider = UFortAnalytics::CreateAnalyticsProvider(FString(), FString(), FString(FPlatformProperties::IniPlatformName()), EAnalyticsProviderType::EsportsServerGameInstance, FString());
            (*ServerEsportsAnalyticsProvider)->StartSession();
        }

        if (auto LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            if (auto PlayerController = LocalPlayer->GetPlayerController())
            {
                if (auto Pawn = PlayerController->GetPawn())
                    Pawn->Destroy(true);

                PlayerController->Destroy(true);
            }

            LocalPlayer->AddToRoot();
            GameInstance->GetLocalPlayers().RemoveSingle(LocalPlayer);
        }

        TCHAR Playlist[256];
        wcscpy_s(Playlist, Config::Playlist);

        if (auto Arg = wcsstr(FCommandLine::Get(), TEXT("-playlist=")))
        {
            Arg = wcschr(Arg, '=') + 1;
            wcsncpy_s(Playlist, Arg, wcscspn(Arg, TEXT(" ")));
        }

        TCHAR Options[128] = {};

        if (auto PlaylistClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))
        {
            if (auto PlaylistData = StaticLoadObject(PlaylistClass, nullptr, Playlist))
            {
                auto Value = [&](const TCHAR* Name)
                {
                    return *PlaylistClass->FindPropertyByName(FName(Name))->ContainerPtrToValuePtr<int32>(PlaylistData);
                };

                swprintf_s(Options, TEXT("playlistId=%d?MaxPlayers=%d?MaxTeamSize=%d"), Value(TEXT("PlaylistId")), Value(TEXT("MaxPlayers")), Value(TEXT("MaxTeamSize")));
            }
        }
        else
        {
            int32 PlaylistId = wcsstr(Playlist, TEXT("Playlist_DefaultDuo")) ? 10 : wcsstr(Playlist, TEXT("Playlist_DefaultSquad")) ? 9 : 2;
            int32 MaxTeamCount;
            int32 MaxTeamSize;
            int32 MaxSquadSize;
            int32 MaxPlayers;

            if (UFortPlaylistManager::GetMaxTeamInfoForPlaylist(nullptr, PlaylistId, MaxTeamCount, MaxTeamSize, MaxSquadSize, MaxPlayers))
                swprintf_s(Options, TEXT("playlistId=%d?MaxPlayers=%d?MaxTeamSize=%d"), PlaylistId, MaxPlayers, MaxTeamSize);
            else
                swprintf_s(Options, TEXT("playlistId=%d"), PlaylistId);
        }

        auto BRMapFullName = UFortGlobals::Get()->GetBRMapFullName();

        UGameplayStatics::OpenLevel(GameInstance, FName(BRMapFullName ? **BRMapFullName : Config::Map), true, FString(Options));
    }
    else if (auto World = GetGameInstance()->GetWorld())
    {
        static AFortGameModeAthena* AssignedGameMode = nullptr;

        auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());

        if (GameMode && GameMode != AssignedGameMode && GameMode->GetMatchState() == MatchState::WaitingToStart)
        {
            if (!World->GetNetDriver())
            {
                FURL URL{};
                World->Listen(URL);
            }

            TCHAR WarmupRequiredPlayerCount[16];
            swprintf_s(WarmupRequiredPlayerCount, TEXT("%d"), Config::WarmupRequiredPlayerCount);

            TMap<FString, FString> MatchOptions;
            MatchOptions.Add(TEXT("match.option.warmupRequiredPlayerCount"), WarmupRequiredPlayerCount);

            GameMode->OnGivenMatchAssignment(MatchOptions);
            GameMode->GetWarmupRequiredPlayerCount() = Config::WarmupRequiredPlayerCount;
            AssignedGameMode = GameMode;
        }

        ABGAConsumableSpawner::TickPendingSpawns();
        ABuildingItemCollectorActor::TickPendingOutputs();
        ABuildingGameplayActorSpawnMachine::TickPendingResurrections();
        UFortGameplayAbilityAthena_PeriodicItemGrant::TickItemAwardTimers();
        UFortServerBotManagerAthena::TickBotBrainActivationTimers();
        UFortControllerComponent_TransientQuests::TickPlayerBountyThreatLevelUpdates();
        ABuildingGameplayActorTransientQuest::TickInteractionTextUpdates();

        if (UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0 && World->GetGameState())
        {
            if (auto LivingWorldManager = (UFortAthenaLivingWorldManager*)World->GetGameState()->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()))
                LivingWorldManager->TickComponent(DeltaSeconds);
        }

        if (auto GameState = Cast<AFortGameStateAthena>(World->GetGameState()))
        {
            switch (GameState->GetGamePhase())
            {
            case EAthenaGamePhase::Aircraft:
            case EAthenaGamePhase::SafeZones:
                State = TEXT("Match Started");

                if (Config::bRestartAfterMatch && GameMode == AssignedGameMode && !GameState->GetPlayerArray().Num() && GameMode->GetFortGameSession())
                {
                    AssignedGameMode = nullptr;
                    GameMode->GetFortGameSession()->Restart();
                }

                break;
            case EAthenaGamePhase::EndGame:
                State = TEXT("Match Ended");
                AssignedGameMode = nullptr;
                break;
            default:
                State = TEXT("Joinable");
                break;
            }
        }
    }

    static const FString Build = []
    {
        auto& Version = FEngineVersion::Current();
        auto Branch = Version.GetBranch();
        auto Release = wcsstr(*Branch, TEXT("Release-"));

        TCHAR Label[64];

        if (Release && iswdigit(Release[8]))
            swprintf_s(Label, TEXT("FN %s, UE %hu.%hu"), Release + 8, Version.GetMajor(), Version.GetMinor());
        else
            swprintf_s(Label, TEXT("CL %u, UE %hu.%hu"), Version.GetChangelist(), Version.GetMajor(), Version.GetMinor());

        return FString(Label);
    }();

    static const TCHAR* LastState = nullptr;

    if (State != LastState)
    {
        TCHAR Title[128];
        swprintf_s(Title, TEXT("Velora (%s): %s"), *Build, State);
        SetConsoleTitleW(Title);

        LastState = State;
    }

    Original(DeltaSeconds, bIdleMode);
}
