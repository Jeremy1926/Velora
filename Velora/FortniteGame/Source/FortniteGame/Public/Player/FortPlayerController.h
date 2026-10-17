#pragma once

#include "GameFramework/PlayerController.h"

class AFortQuickBars;

class AFortPlayerController : public APlayerController
{
public:
    class FOnMcpProfilesInitialized;

    bool InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized);

    AFortQuickBars*& GetQuickBars();
};
