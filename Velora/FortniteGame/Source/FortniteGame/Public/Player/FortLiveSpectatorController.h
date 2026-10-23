#pragma once

#include "Player/FortPlayerControllerSpectating.h"

class AFortLiveSpectatorController : public AFortPlayerControllerSpectating
{
public:
    static UClass* StaticClass();

    AFortPlayerState* GetFollowedPlayerState() const;

    void GetRemoteClientInfos(TArray<AFortBroadcastRemoteClientInfo*>& OutRemoteClientInfos) const;
};
