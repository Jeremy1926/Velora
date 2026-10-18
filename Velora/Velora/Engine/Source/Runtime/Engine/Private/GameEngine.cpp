#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Online/FortGameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "ContentStreaming.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "HAL/PlatformProperties.h"
#include "Misc/EngineVersion.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "Online/FortPlaylistManager.h"
#include "FortGameInstance.h"
#include "Analytics/FortAnalytics.h"
#include "CoreGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

static auto Tick = Detour(TEXT("UGameEngine::Tick"), Scanner::String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)")).Start().Get(), &UGameEngine::Tick);

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    const TCHAR* State = TEXT("Loading");

    if (!GIsServer)
    {
        UE_LOG(LogVelora, Display, TEXT("Starting dedicated server"));

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

        if (auto LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            LocalPlayer->AddToRoot();
            GameInstance->GetLocalPlayers().RemoveSingle(LocalPlayer);
        }

        int32 PlaylistId = std::array<int32, 3>{ 2, 10, 9 }[(uint8)Config::Playlist];
        FParse::Value(FCommandLine::Get(), TEXT("playlistId="), PlaylistId);

        int32 MaxTeamCount;
        int32 MaxTeamSize;
        int32 MaxSquadSize;
        int32 MaxPlayers;
        TCHAR Options[128];

        if (UFortPlaylistManager::GetMaxTeamInfoForPlaylist(PlaylistId, MaxTeamCount, MaxTeamSize, MaxSquadSize, MaxPlayers))
            swprintf_s(Options, TEXT("playlistId=%d?MaxPlayers=%d?MaxTeamSize=%d"), PlaylistId, MaxPlayers, MaxTeamSize);
        else
            swprintf_s(Options, TEXT("playlistId=%d"), PlaylistId);

        UE_LOG(LogVelora, Display, TEXT("Loading %s?%s"), Config::Map, Options);

        UGameplayStatics::OpenLevel(GameInstance, FName(Config::Map), true, FString(Options));
    }
    else if (auto World = GetGameInstance()->GetWorld())
    {
        static AFortGameModeAthena* AssignedGameMode = nullptr;
        static bool bWinnerLogged = false;

        auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());

        if (GameMode && GameMode != AssignedGameMode && GameMode->GetMatchState() == MatchState::WaitingToStart)
        {
            TCHAR WarmupRequiredPlayerCount[16];
            swprintf_s(WarmupRequiredPlayerCount, TEXT("%d"), Config::WarmupRequiredPlayerCount);

            TMap<FString, FString> MatchOptions;
            MatchOptions.Add(TEXT("match.option.warmupRequiredPlayerCount"), WarmupRequiredPlayerCount);

            GameMode->OnGivenMatchAssignment(MatchOptions);
            AssignedGameMode = GameMode;
            bWinnerLogged = false;

            UE_LOG(LogVelora, Display, TEXT("Match assigned, warmup starts with %d player%s"), Config::WarmupRequiredPlayerCount, Config::WarmupRequiredPlayerCount == 1 ? TEXT("") : TEXT("s"));
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
                    UE_LOG(LogVelora, Display, TEXT("Everyone left during the match"));

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

            static int32 LastPlayersLeft = -1;

            if (GameState->GetPlayersLeft() != LastPlayersLeft)
            {
                LastPlayersLeft = GameState->GetPlayersLeft();

                UE_LOG(LogVelora, Display, TEXT("%d player%s left alive"), LastPlayersLeft, LastPlayersLeft == 1 ? TEXT("") : TEXT("s"));
            }

            static int32 LastTeamsLeft = -1;

            if (GameState->GetTeamsLeft() != LastTeamsLeft)
            {
                LastTeamsLeft = GameState->GetTeamsLeft();

                if (LastTeamsLeft != GameState->GetPlayersLeft())
                    UE_LOG(LogVelora, Display, TEXT("%d team%s left alive"), LastTeamsLeft, LastTeamsLeft == 1 ? TEXT("") : TEXT("s"));
            }

            if (!bWinnerLogged && GameState->GetWinningPlayerName().Len())
            {
                bWinnerLogged = true;

                UE_LOG(LogVelora, Display, TEXT("%s won the match"), *GameState->GetWinningPlayerName());
            }

            static TArray<APlayerState*> Players;
            static TArray<FString> PlayerNames;

            auto& PlayerArray = GameState->GetPlayerArray();

            for (auto PlayerState : PlayerArray)
            {
                if (PlayerState && !Players.Contains(PlayerState))
                {
                    Players.Add(PlayerState);
                    PlayerNames.Add(PlayerState->GetPlayerName());

                    UE_LOG(LogVelora, Display, TEXT("%s joined, %d player%s online"), *PlayerNames[PlayerNames.Num() - 1], Players.Num(), Players.Num() == 1 ? TEXT("") : TEXT("s"));
                }
            }

            for (int32 Index = Players.Num() - 1; Index >= 0; Index--)
            {
                if (!PlayerArray.Contains(Players[Index]))
                {
                    const int32 Online = Players.Num() - 1;

                    UE_LOG(LogVelora, Display, TEXT("%s left, %d player%s online"), *PlayerNames[Index], Online, Online == 1 ? TEXT("") : TEXT("s"));

                    Players.RemoveAt(Index);
                    PlayerNames.RemoveAt(Index);
                }
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

        UE_LOG(LogVelora, Display, TEXT("State: %s"), State);

        LastState = State;
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
