#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "ContentStreaming.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "HAL/PlatformProperties.h"
#include "Misc/EngineVersion.h"
#include "FortGameInstance.h"
#include "Analytics/FortAnalytics.h"
#include "CoreGlobals.h"
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

        UE_LOG(LogVelora, Display, TEXT("Loading /Game/Athena/Maps/Athena_Terrain"));

        UGameplayStatics::OpenLevel(GameInstance, FName(TEXT("/Game/Athena/Maps/Athena_Terrain")));
    }
    else if (auto World = GetGameInstance()->GetWorld())
    {
        static AFortGameModeAthena* AssignedGameMode = nullptr;

        auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());

        if (GameMode && GameMode != AssignedGameMode && GameMode->GetMatchState() == MatchState::WaitingToStart)
        {
            TMap<FString, FString> MatchOptions;
            MatchOptions.Add(TEXT("match.option.warmupRequiredPlayerCount"), TEXT("1"));

            GameMode->OnGivenMatchAssignment(MatchOptions);
            AssignedGameMode = GameMode;

            UE_LOG(LogVelora, Display, TEXT("Match assigned, warmup starts with 1 player"));
        }

        if (auto GameState = Cast<AFortGameStateAthena>(World->GetGameState()))
        {
            switch (GameState->GetGamePhase())
            {
            case EAthenaGamePhase::Aircraft:
            case EAthenaGamePhase::SafeZones:
                State = TEXT("Match Started");
                break;
            case EAthenaGamePhase::EndGame:
                State = TEXT("Match Ended");
                break;
            default:
                State = TEXT("Joinable");
                break;
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

    static const TCHAR* LastState = nullptr;

    if (State != LastState)
    {
        auto& Version = FEngineVersion::Current();

        TCHAR Title[128];
        swprintf_s(Title, TEXT("Velora UE %hu.%hu | %s"), Version.GetMajor(), Version.GetMinor(), State);
        SetConsoleTitleW(Title);

        UE_LOG(LogVelora, Display, TEXT("State: %s"), State);

        LastState = State;
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
