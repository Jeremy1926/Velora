#pragma once

#include "Pawns/FortPawn.h"
#include "Math/Vector.h"

class AFortPickup;

class AFortPlayerPawn : public AFortPawn
{
public:
    void OnBlueprintPlace();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);
};
