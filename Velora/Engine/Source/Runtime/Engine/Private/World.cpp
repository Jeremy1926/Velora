#include "pch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "CoreGlobals.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

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

UAISystemBase* UWorld::GetAISystem() const
{
    static auto AISystem = GetClass()->FindPropertyByName(FName(TEXT("AISystem")));

    return *AISystem->ContainerPtrToValuePtr<UAISystemBase*>(this);
}

UGameInstance* UWorld::GetGameInstance() const
{
    static auto OwningGameInstance = GetClass()->FindPropertyByName(FName(TEXT("OwningGameInstance")));

    return *OwningGameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

AGameNetworkManager* UWorld::GetNetworkManager() const
{
    static auto NetworkManager = GetClass()->FindPropertyByName(FName(TEXT("NetworkManager")));

    return *NetworkManager->ContainerPtrToValuePtr<AGameNetworkManager*>(this);
}

ULevel* UWorld::GetPersistentLevel() const
{
    static auto PersistentLevel = GetClass()->FindPropertyByName(FName(TEXT("PersistentLevel")));

    return *PersistentLevel->ContainerPtrToValuePtr<ULevel*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingVisibility() const
{
    static auto CurrentLevelPendingVisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingVisibility")));

    return *CurrentLevelPendingVisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

const TArray<ULevelStreaming*>& UWorld::GetStreamingLevels() const
{
    static auto StreamingLevels = GetClass()->FindPropertyByName(FName(TEXT("StreamingLevels")));

    return *StreamingLevels->ContainerPtrToValuePtr<TArray<ULevelStreaming*>>(this);
}

ULevel* UWorld::GetCurrentLevelPendingInvisibility() const
{
    static auto CurrentLevelPendingInvisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingInvisibility")));

    return *CurrentLevelPendingInvisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

float UWorld::GetTimeSeconds() const
{
    static auto TimeSeconds = Anchor::Offset(TEXT("UWorld::TimeSeconds"))
        .Native(TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds"))
        .Find("movss xmm0, dword ptr [rax+*]")
        .Disp();

    return *(float*)((uint8_t*)this + TimeSeconds);
}

AWorldSettings* UWorld::GetWorldSettings(bool bCheckStreamingPesistent, bool bChecked) const
{
    static auto GetWorldSettings = Anchor::Function<AWorldSettings* (*)(const UWorld*, bool, bool)>(TEXT("UWorld::GetWorldSettings"))
        .Native(TEXT("/Script/Engine.GameplayStatics:GetGlobalTimeDilation"))
        .Call(2)
        .Get();

    return GetWorldSettings(this, bCheckStreamingPesistent, bChecked);
}

UNetDriver* UWorld::GetNetDriver() const
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    return *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

UNetDriver* UWorld::GetDemoNetDriver() const
{
    static auto DemoNetDriver = GetClass()->FindPropertyByName(FName(TEXT("DemoNetDriver")));

    return *DemoNetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

ENetMode UWorld::InternalGetNetMode() const
{
    DETOUR(UWorld, InternalGetNetMode, Anchor::Hook()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, rax; call 0x*")
        .Call()
        .Or()
        .String(TEXT("Failed to determine if OSS is server in PIE, OSS requests will fail"))
        .Start()
        .Find("mov rcx, *; call 0x*; mov ebx, eax; lea eax, [rbx-0x1]")
        .Call()
        .Inline());

    if (GetNetDriver() != nullptr)
    {
        return GetNetDriver()->GetNetMode();
    }

    return Original();
}

void UWorld::SetNetDriver(UNetDriver* NewDriver)
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this) = NewDriver;
}

void UWorld::SetNextSwitchCountdown(float InNextSwitchCountdown)
{
    static auto NextSwitchCountdown = Anchor::Offset(TEXT("UWorld::NextSwitchCountdown"))
        .String(TEXT("Server switch level: %s"))
        .Start()
        .Find("subss *")
        .Find("movss [*], xmm*")
        .Or()
        .String(TEXT("Server switch level: %s"))
        .Last("subss *")
        .Find("movss [*], xmm*")
        .Disp();

    if (NextSwitchCountdown > 0)
        *(float*)((uint8_t*)this + NextSwitchCountdown) = InNextSwitchCountdown;
}

FLevelCollection* UWorld::FindCollectionByType(const ELevelCollectionType InType)
{
    static auto FindCollectionByType = Anchor::Function<FLevelCollection* (*)(UWorld*, ELevelCollectionType)>(TEXT("UWorld::FindCollectionByType"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Find("xor edx, edx; mov rcx, *")
        .Call()
        .Get();

    return FindCollectionByType(this, InType);
}

void UWorld::InitializeActorsForPlay(const FURL& InURL, bool bResetTime, FRegisterComponentContext* Context)
{
    DETOUR(UWorld, InitializeActorsForPlay, Anchor::Hook()
        .String(TEXT("Bringing up level for play took: %f"))
        .Start()
        .Optional()
        .Inline());

    if (!GIsClient && !GetNetDriver())
        Listen(const_cast<FURL&>(InURL));

    Original(InURL, bResetTime, Context);
}

bool UWorld::Listen(FURL& InURL)
{
    DETOUR(UWorld, Listen, Anchor::Hook()
        .String(TEXT("LoadMap: failed to Listen(%s)"))
        .Last("call 0x*; test al, al")
        .Optional()
        .AtCall());

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
        static auto& LogWorld = *Anchor::Data<FLogCategoryBase*>(TEXT("LogWorld"))
            .String(TEXT("Bringing %s up for play (max tick rate %i) at %s"))
            .Last("movzx *, byte ptr [0x*]")
            .Target()
            .Get();

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

    return true;
}

bool UWorld::AreActorsInitialized() const
{
    static auto AreActorsInitialized = Anchor::Function<bool (*)(const UWorld*)>(TEXT("UWorld::AreActorsInitialized"))
        .Pattern("33 C0 38 81 ? ? 00 00 7D ? 48 8B 49 30 48 85 C9 74 ? 39 81 ? ? 00 00 74 ? B0 01 C3")
        .Get();

    return AreActorsInitialized(this);
}

UWorld::FOnActorsInitialized& UWorld::GetOnActorsInitialized()
{
    static auto OnActorsInitialized = Anchor::Offset(TEXT("UWorld::OnActorsInitialized"))
        .String(TEXT("Bringing up level for play took: %f"))
        .Last("lea rcx, [*+0x*]; lea rdx, [*]; mov [*], *; mov [*], *; call 0x*; lea rdx, [*]; lea rcx, [0x*]; call 0x*")
        .Disp();

    return *(FOnActorsInitialized*)((uint8*)this + OnActorsInitialized);
}
