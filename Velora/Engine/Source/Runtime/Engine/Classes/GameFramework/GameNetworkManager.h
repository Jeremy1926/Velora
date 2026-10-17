#pragma once

#include "GameFramework/Actor.h"

class AGameNetworkManager : public AActor
{
public:
    bool IsInLowBandwidthMode();
};
