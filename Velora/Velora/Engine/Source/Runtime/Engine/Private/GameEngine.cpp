#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Athena/FortGameModeAthena.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "ContentStreaming.h"
#include "Interfaces/IAnalyticsProvider.h"
#include "HAL/PlatformProperties.h"
#include "FortGameInstance.h"
#include "Analytics/FortAnalytics.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto Tick = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)")))->BeginAddress, &UGameEngine::Tick);

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
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

        if (auto LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            LocalPlayer->AddToRoot();
            GameInstance->GetLocalPlayers().RemoveSingle(LocalPlayer);
        }

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
        }
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
