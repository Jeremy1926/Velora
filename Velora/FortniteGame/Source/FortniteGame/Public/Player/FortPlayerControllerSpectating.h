#pragma once

#include "Player/FortPlayerControllerGameplay.h"

class AFortBroadcastRemoteClientInfo;
class AFortPlayerState;
class APawn;

class AFortPlayerControllerSpectating : public AFortPlayerControllerGameplay
{
public:
    AFortPlayerState* GetFollowedPlayer() const;

    APawn* GetFollowedPlayerPawn(bool bAlwaysReturnPlayer) const;

    AFortBroadcastRemoteClientInfo* GetFollowedPlayerRemoteClientInfo() const;
};
