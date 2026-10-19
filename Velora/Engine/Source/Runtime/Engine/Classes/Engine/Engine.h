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
    float GetNetClientTicksPerSecond() const;

    float GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing = true) const;

    UGameViewportClient* GetGameViewport() const;

    UClass* GetConsoleClass() const;

    void BroadcastNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString = TEXT(""));

    bool CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition);

    UNetDriver* FindNamedNetDriver(UWorld* InWorld, FName NetDriverName);
};

extern UEngine*& GEngine;
