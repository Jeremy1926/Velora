#include "pch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

static FLogCategoryBase& LogWorld = *(FLogCategoryBase*)Scanner::String(TEXT("Bringing %s up for play (max tick rate %i) at %s")).Last("movzx *, byte ptr [0x*]").Target();

static auto Listen = DetourCall(TEXT("UWorld::Listen"), Scanner::String(TEXT("LoadMap: failed to Listen(%s)")).Last("call 0x*; test al, al").Get(), &UWorld::Listen);

static auto InternalGetNetMode = Detour(TEXT("UWorld::InternalGetNetMode"), Scanner::String(TEXT("UGameplayStatics::SpawnEmitterAttached: NULL AttachComponent specified!")).Call(3).Get(), &UWorld::InternalGetNetMode);

AGameModeBase* UWorld::GetAuthGameMode() const
{
    static auto AuthorityGameMode = GetClass()->FindPropertyByName(FName(TEXT("AuthorityGameMode")));

    return *AuthorityGameMode->ContainerPtrToValuePtr<AGameModeBase*>(this);
}

AGameStateBase* UWorld::GetGameState() const
{
    static auto GameState = GetClass()->FindPropertyByName(FName(TEXT("GameState")));

    return *GameState->ContainerPtrToValuePtr<AGameStateBase*>(this);
}

AGameNetworkManager* UWorld::GetNetworkManager() const
{
    static auto NetworkManager = GetClass()->FindPropertyByName(FName(TEXT("NetworkManager")));

    return *NetworkManager->ContainerPtrToValuePtr<AGameNetworkManager*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingVisibility() const
{
    static auto CurrentLevelPendingVisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingVisibility")));

    return *CurrentLevelPendingVisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingInvisibility() const
{
    static auto CurrentLevelPendingInvisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingInvisibility")));

    return *CurrentLevelPendingInvisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

float UWorld::GetTimeSeconds() const
{
    static auto TimeSeconds = Scanner::Native(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds")).Find("movss xmm0, dword ptr [rax+*]").Disp();

    return *(float*)((uint8_t*)this + TimeSeconds);
}

AWorldSettings* UWorld::GetWorldSettings(bool bCheckStreamingPesistent, bool bChecked) const
{
    static auto GetWorldSettings = (AWorldSettings* (*)(const UWorld*, bool, bool))Scanner::Native(TEXT("/Script/Engine.GameplayStatics:GetGlobalTimeDilation")).Call(2).Get();

    return GetWorldSettings(this, bCheckStreamingPesistent, bChecked);
}

UNetDriver* UWorld::GetNetDriver() const
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    return *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

ENetMode UWorld::InternalGetNetMode() const
{
    if (GetNetDriver() != nullptr)
    {
        return GetNetDriver()->GetNetMode();
    }

    return (this->*::InternalGetNetMode)();
}

void UWorld::SetNetDriver(UNetDriver* NewDriver)
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this) = NewDriver;
}

void UWorld::SetNextSwitchCountdown(float InNextSwitchCountdown)
{
    static auto NextSwitchCountdown = Scanner::String(TEXT("Server switch level: %s")).Start().Find("subss *").Find("movss [*], xmm*").Disp();

    *(float*)((uint8_t*)this + NextSwitchCountdown) = InNextSwitchCountdown;
}

FLevelCollection* UWorld::FindCollectionByType(const ELevelCollectionType InType)
{
    static auto FindCollectionByType = (FLevelCollection* (*)(UWorld*, ELevelCollectionType))Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("xor edx, edx").Call().Get();

    return FindCollectionByType(this, InType);
}

bool UWorld::Listen(FURL& InURL)
{
    if (GetNetDriver())
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverAlreadyExists);
        return false;
    }

    if (GEngine->CreateNamedNetDriver(this, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        SetNetDriver(GEngine->FindNamedNetDriver(this, NAME_GameNetDriver));
        GetNetDriver()->SetWorld(this);

        FLevelCollection* const SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(GetNetDriver());

        FLevelCollection* const StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(GetNetDriver());
    }

    if (GetNetDriver() == nullptr)
    {
        GEngine->BroadcastNetworkFailure(this, nullptr, ENetworkFailure::NetDriverCreateFailure);
        return false;
    }

    FString Error;

    InURL.Port = Config::Port;

    if (!GetNetDriver()->InitListen(this, InURL, false, Error))
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverListenFailure, Error);
        UE_LOG(LogWorld, Log, TEXT("Failed to listen: %s"), *Error);
        GetNetDriver()->SetWorld(nullptr);
        SetNetDriver(nullptr);

        FLevelCollection* SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(nullptr);

        FLevelCollection* StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(nullptr);

        return false;
    }

    static bool LanPlay = FParse::Param(FCommandLine::Get(), TEXT("lanplay"));

    if (!LanPlay && (GetNetDriver()->GetMaxInternetClientRate() < GetNetDriver()->GetMaxClientRate()) && (GetNetDriver()->GetMaxInternetClientRate() > 2500))
        GetNetDriver()->SetMaxClientRate(GetNetDriver()->GetMaxInternetClientRate());

    SetNextSwitchCountdown(GetNetDriver()->GetServerTravelPause());

    UE_LOG(LogVelora, Display, TEXT("Listening on port %d"), InURL.Port);

    return true;
}
