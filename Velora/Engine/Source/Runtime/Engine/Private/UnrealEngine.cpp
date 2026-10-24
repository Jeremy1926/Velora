#include "pch.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UEngine*& GEngine = *Anchor::Data<UEngine**>(TEXT("GEngine"))
    .String(TEXT("STAT_WorldTickTime"))
    .Find("mov rcx, [0x*]")
    .Target()
    .Or()
    .String(TEXT("RecordReplay: failed to create demo net driver!"))
    .Last("mov rcx, [0x*]")
    .Target()
    .Get();

void FWorldContext::SetRunAsDedicated(bool bInRunAsDedicated)
{
    static auto RunAsDedicated = Anchor::Offset(TEXT("FWorldContext::RunAsDedicated"))
        .Pattern("48 8B 41 30 48 85 C0 74 ? 0F B6 80 ? ? 00 00 C3")
        .Find("movzx *, byte ptr [*+0x*]")
        .Or()
        .Pattern("48 8B 51 30 33 C0 48 85 D2 74 ? 8A 82 ? ? 00 00 C3")
        .Find("mov al, [*+0x*]")
        .Disp();

    if (RunAsDedicated > 0)
        *(bool*)((uint8_t*)this + RunAsDedicated) = bInRunAsDedicated;
}

float UEngine::GetNetClientTicksPerSecond() const
{
    static auto NetClientTicksPerSecond = GetClass()->FindPropertyByName(FName(TEXT("NetClientTicksPerSecond")));

    return *NetClientTicksPerSecond->ContainerPtrToValuePtr<float>(this);
}

float UEngine::GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing) const
{
    static auto GetMaxTickRate = Anchor::VirtualFunction<float (*)(const UEngine*, float, bool)>(TEXT("UEngine::GetMaxTickRate"))
        .String(TEXT("Bringing %s up for play (max tick rate %i) at %s"))
        .Last("call [*+0x*]")
        .Get();

    return GetMaxTickRate(this, DeltaTime, bAllowFrameRateSmoothing);
}

UGameViewportClient* UEngine::GetGameViewport() const
{
    static auto GameViewport = GetClass()->FindPropertyByName(FName(TEXT("GameViewport")));

    return *GameViewport->ContainerPtrToValuePtr<UGameViewportClient*>(this);
}

UClass* UEngine::GetConsoleClass() const
{
    static auto ConsoleClass = GetClass()->FindPropertyByName(FName(TEXT("ConsoleClass")));

    return *ConsoleClass->ContainerPtrToValuePtr<UClass*>(this);
}

UObject* UEngine::GetGameSingleton() const
{
    static auto GameSingleton = GetClass()->FindPropertyByName(FName(TEXT("GameSingleton")));

    return *GameSingleton->ContainerPtrToValuePtr<UObject*>(this);
}

void UEngine::BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    static auto NetworkFailureEvent = (int32)Anchor::Offset(TEXT("UEngine::NetworkFailureEvent"))
        .String(TEXT("Beacon close from unexpected control message"))
        .Start()
        .Find("mov r9d, 0x6")
        .Find("call 0x*")
        .LastSince("add rcx, 0x*", "mov rcx, [0x*]")
        .Optional()
        .Imm();

    static auto Broadcast = Anchor::Function<void (*)(void*, UWorld*, UNetDriver*, ENetworkFailure::Type, const FString&)>(TEXT("UEngine::BroadcastNetworkFailure"))
        .String(TEXT("Beacon close from unexpected control message"))
        .Start()
        .Find("mov r9d, 0x6")
        .Find("call 0x*")
        .Target()
        .Get();

    Broadcast((uint8_t*)this + NetworkFailureEvent, World, NetDriver, FailureType, ErrorString);
}

bool UEngine::CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition)
{
    static auto CreateNamedNetDriver = Anchor::Function<bool (*)(UEngine*, UWorld*, FName, FName)>(TEXT("UEngine::CreateNamedNetDriver"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Last("call 0x*")
        .Target()
        .Get();

    return CreateNamedNetDriver(this, InWorld, NetDriverName, NetDriverDefinition);
}

UNetDriver* UEngine::FindNamedNetDriver(UWorld* InWorld, FName NetDriverName)
{
    static auto FindNamedNetDriver = Anchor::Function<UNetDriver* (*)(UEngine*, UWorld*, FName)>(TEXT("UEngine::FindNamedNetDriver"))
        .Pattern("40 53 48 83 EC 20 41 8B D8 E8 ? ? ? ? 48 85 C0 74 ? 48 63 88 ? ? 00 00 85 C9 7E ? 48 8B 90 ? ? 00 00")
        .Or()
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Find("mov rcx, [0x*]")
        .Call()
        .Get();

    return FindNamedNetDriver(this, InWorld, NetDriverName);
}

void UEngine::ForceGarbageCollection(bool bFullPurge)
{
    static auto ForceGarbageCollection = Anchor::Function<void (*)(UEngine*, bool)>(TEXT("UEngine::ForceGarbageCollection"))
        .Pattern("48 89 5C 24 08 57 48 83 EC 20 40 8A FA 48 8B D9 E8 ? ? ? ? F3 0F 58 05 ? ? ? ? 32 C0 F3 0F 11 83 ? ? ? ? 38 83 ? ? ? ? 75 ? 40 84 FF 75 ? 88 83")
        .Or()
        .Pattern("48 89 5C 24 08 57 48 83 EC 20 40 8A FA 48 8B D9 E8 ? ? ? ? F3 0F 58 05 ? ? ? ? 32 C0 F3 0F 11 83 ? ? ? ? 38 83 ? ? ? ? 74 ? B0 01 88 83")
        .Get();

    ForceGarbageCollection(this, bFullPurge);
}
