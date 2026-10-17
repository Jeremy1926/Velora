#pragma once

#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Engine/PendingNetGame.h"

class AGameNetworkManager;
class AGameStateBase;
class AWorldSettings;
class ULevel;
class UNetDriver;
struct FURL;

struct FLevelCollection
{
public:
    void SetNetDriver(UNetDriver* const InNetDriver)
    {
        NetDriver = InNetDriver;
    }

private:
    ELevelCollectionType CollectionType;
    AGameStateBase* GameState;
    UNetDriver* NetDriver;
};

class UWorld final : public UObject, public FNetworkNotify
{
public:
    AGameNetworkManager* GetNetworkManager() const;

    ULevel* GetCurrentLevelPendingVisibility() const;

    ULevel* GetCurrentLevelPendingInvisibility() const;

    float GetTimeSeconds() const;

    AWorldSettings* GetWorldSettings(bool bCheckStreamingPesistent = false, bool bChecked = true) const;

    UNetDriver* GetNetDriver() const;

    void SetNetDriver(UNetDriver* NewDriver);

    void SetNextSwitchCountdown(float InNextSwitchCountdown);

    FLevelCollection* FindCollectionByType(const ELevelCollectionType InType);

    bool Listen(FURL& InURL);
};
