#include "pch.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UEngine*& GEngine = *(UEngine**)Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Last("mov rcx, [0x*]").Target(TEXT("GEngine"));

void FWorldContext::SetRunAsDedicated(bool bInRunAsDedicated)
{
    static auto RunAsDedicated = Scanner::Pattern("48 8B 41 30 48 85 C0 74 ? 0F B6 80 ? ? 00 00 C3").Find("movzx *, byte ptr [*+0x*]").Disp(TEXT("FWorldContext::RunAsDedicated"));

    *(bool*)((uint8_t*)this + RunAsDedicated) = bInRunAsDedicated;
}

float UEngine::GetNetClientTicksPerSecond() const
{
    static auto NetClientTicksPerSecond = GetClass()->FindPropertyByName(FName(TEXT("NetClientTicksPerSecond")));

    return *NetClientTicksPerSecond->ContainerPtrToValuePtr<float>(this);
}

float UEngine::GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing) const
{
    static auto GetMaxTickRate = Scanner::String(TEXT("Bringing %s up for play (max tick rate %i) at %s")).Last("call [*+0x*]").Slot(TEXT("UEngine::GetMaxTickRate"));

    return Scanner::Virtual<float (*)(const UEngine*, float, bool)>(this, GetMaxTickRate)(this, DeltaTime, bAllowFrameRateSmoothing);
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

void UEngine::BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    static auto Call = Scanner::String(TEXT("Beacon close from unexpected control message")).Start().Find("mov r9d, 0x6").Find("call 0x*");

    static auto NetworkFailureEvent = []
    {
        auto Engine = Call.Last("mov rcx, [0x*]");
        auto Event = Call.Last("add rcx, 0x*");

        return Event.Get() > Engine.Get() ? (int32)Event.Imm(TEXT("UEngine::NetworkFailureEvent")) : 0;
    }();

    static auto Broadcast = Scanner::Function<void (*)(void*, UWorld*, UNetDriver*, ENetworkFailure::Type, const FString&)>(Call.Target(TEXT("UEngine::BroadcastNetworkFailure")));

    Broadcast((uint8_t*)this + NetworkFailureEvent, World, NetDriver, FailureType, ErrorString);
}

bool UEngine::CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition)
{
    static auto CreateNamedNetDriver = Scanner::Function<bool (*)(UEngine*, UWorld*, FName, FName)>(Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Last("call 0x*").Target(TEXT("UEngine::CreateNamedNetDriver")));

    return CreateNamedNetDriver(this, InWorld, NetDriverName, NetDriverDefinition);
}

UNetDriver* UEngine::FindNamedNetDriver(UWorld* InWorld, FName NetDriverName)
{
    static auto FindNamedNetDriver = Scanner::Function<UNetDriver* (*)(UEngine*, UWorld*, FName)>(Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Find("mov rcx, [0x*]; call 0x*").Call().Get(TEXT("UEngine::FindNamedNetDriver")));

    return FindNamedNetDriver(this, InWorld, NetDriverName);
}
