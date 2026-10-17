#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"

class FNetworkNotify;
class UWorld;
struct FURL;

class UNetDriver : public UObject
{
public:
    int32 GetMaxInternetClientRate() const;

    int32 GetMaxClientRate() const;

    void SetMaxClientRate(int32 InMaxClientRate);

    float GetServerTravelPause() const;

    bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);

    void SetWorld(UWorld* InWorld);
};
