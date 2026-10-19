#pragma once

#include "Player/FortPlayerControllerGameplay.h"

class AController;
class APawn;

class AFortPlayerControllerZone : public AFortPlayerControllerGameplay
{
public:
    typedef AFortPlayerControllerGameplay Super;

    static UClass* StaticClass();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void OnPawnRevived(AController* EventInstigator);

    void ApplyDeathDurabilityPenalty(float DurabilityPenalty);
};
