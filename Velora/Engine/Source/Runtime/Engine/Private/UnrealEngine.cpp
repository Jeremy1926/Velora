#include "pch.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UEngine*& GEngine = *(UEngine**)Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Last("mov rcx, [0x*]").Target();

void FWorldContext::SetRunAsDedicated(bool bInRunAsDedicated)
{
    static auto RunAsDedicated = Scanner::String(TEXT("TeamChatroom")).Start().Find("call 0x*; test al, al; jnz *").Call().Find("movzx *, byte ptr [*+0x*]").Disp();

    *(bool*)((uint8_t*)this + RunAsDedicated) = bInRunAsDedicated;
}

float UEngine::GetNetClientTicksPerSecond() const
{
    static auto NetClientTicksPerSecond = GetClass()->FindPropertyByName(FName(TEXT("NetClientTicksPerSecond")));

    return *NetClientTicksPerSecond->ContainerPtrToValuePtr<float>(this);
}

float UEngine::GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing) const
{
    static auto GetMaxTickRate = Scanner::String(TEXT("Bringing %s up for play (max tick rate %i) at %s")).Last("call [*+0x*]").Disp();

    auto VTable = *(uint8_t**)this;

    return (*(float (**)(const UEngine*, float, bool))(VTable + GetMaxTickRate))(this, DeltaTime, bAllowFrameRateSmoothing);
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

UEngine::FOnNetworkFailure& UEngine::OnNetworkFailure()
{
    static auto NetworkFailureEvent = Scanner::String(TEXT("Server switch level: %s")).Find("mov r9d, 0x8").Last("lea rcx, [*+0x*]").Disp();

    return *(FOnNetworkFailure*)((uint8_t*)this + NetworkFailureEvent);
}

void UEngine::FOnNetworkFailure::Broadcast(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) const
{
    static auto Broadcast = (void (*)(const FOnNetworkFailure*, UWorld*, UNetDriver*, ENetworkFailure::Type, const FString&))Scanner::String(TEXT("Server switch level: %s")).Find("mov r9d, 0x8").Call().Get();

    Broadcast(this, World, NetDriver, FailureType, ErrorString);
}

bool UEngine::CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition)
{
    static auto CreateNamedNetDriver = (bool (*)(UEngine*, UWorld*, FName, FName))Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Last("call 0x*").Target();

    return CreateNamedNetDriver(this, InWorld, NetDriverName, NetDriverDefinition);
}

UNetDriver* UEngine::FindNamedNetDriver(UWorld* InWorld, FName NetDriverName)
{
    static auto FindNamedNetDriver = (UNetDriver* (*)(UEngine*, UWorld*, FName))Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("mov rcx, [0x*]; call 0x*").Call().Get();

    return FindNamedNetDriver(this, InWorld, NetDriverName);
}
