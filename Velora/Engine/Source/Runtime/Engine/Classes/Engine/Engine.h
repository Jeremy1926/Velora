#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineBaseTypes.h"

class UGameViewportClient;
class UNetDriver;
class UWorld;

struct FWorldContext
{
    void SetRunAsDedicated(bool bInRunAsDedicated);
};

class UEngine : public UObject
{
public:
    class FOnNetworkFailure
    {
    public:
        void Broadcast(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) const;
    };

    UGameViewportClient* GetGameViewport() const;

    UClass* GetConsoleClass() const;

    FOnNetworkFailure& OnNetworkFailure();

    void BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString = TEXT(""))
    {
        OnNetworkFailure().Broadcast(World, NetDriver, FailureType, ErrorString);
    }

    bool CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition);

    UNetDriver* FindNamedNetDriver(UWorld* InWorld, FName NetDriverName);
};

extern UEngine*& GEngine;
