#pragma once

#include "GameFramework/Info.h"
#include "Delegates/Delegate.h"

class AFortPlayerState;

class AFortTeamPrivateInfo : public AInfo
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>& GetNotifyPlayerAdded();

    static TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>& GetNotifyPlayerRemoved();
};
