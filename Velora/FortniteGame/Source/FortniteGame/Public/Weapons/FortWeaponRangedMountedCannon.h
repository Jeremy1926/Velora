#pragma once

#include "Weapons/FortWeapon.h"
#include "Math/Vector.h"

class AFortWeaponRangedMountedCannon : public AFortWeapon
{
public:
    void ServerFireActorInCannon_Implementation(const TGameValue<FVector>& GameLaunchDir);

    bool ShouldFireRiderInCannon(const FVector& LaunchDir, bool bFireRider);
};
