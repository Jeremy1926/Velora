#pragma once

#include "GameFramework/Controller.h"

class APlayerController : public AController
{
public:
    void SendClientAdjustment();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void ServerRestartPlayer_Implementation();
};
