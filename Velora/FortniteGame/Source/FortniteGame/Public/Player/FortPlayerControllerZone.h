#pragma once

#include "Player/FortPlayerControllerGameplay.h"

class AController;

class AFortPlayerControllerZone : public AFortPlayerControllerGameplay
{
public:
    static UClass* StaticClass();

    void OnPawnRevived(AController* EventInstigator);

    void ApplyDeathDurabilityPenalty(float DurabilityPenalty);
};
