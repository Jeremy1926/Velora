#pragma once

#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Engine/PendingNetGame.h"

class AGameStateBase;
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
    UNetDriver* GetNetDriver() const;

    void SetNetDriver(UNetDriver* NewDriver);

    void SetNextSwitchCountdown(float InNextSwitchCountdown);

    FLevelCollection* FindCollectionByType(const ELevelCollectionType InType);

    bool Listen(FURL& InURL);
};
