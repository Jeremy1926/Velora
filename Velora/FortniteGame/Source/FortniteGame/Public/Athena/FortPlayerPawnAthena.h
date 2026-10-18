#pragma once

#include "Pawns/FortPlayerPawn.h"

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
    void OnPlayerPawnReady();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerReviveFromDBNO_Implementation(AController* EventInstigator);

    float GetPickupSpeedMultiplier() const;

    uint8& GetDBNORevivalStacking();
};
