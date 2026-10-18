#pragma once

#include "Pawns/FortPlayerPawn.h"

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
    void OnPlayerPawnReady();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    float GetPickupSpeedMultiplier() const;
};
