#pragma once

#include "GameFramework/Controller.h"

class APlayerController : public AController
{
public:
    void SendClientAdjustment();
};
