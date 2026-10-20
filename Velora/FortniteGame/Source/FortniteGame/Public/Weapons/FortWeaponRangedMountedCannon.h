#pragma once

#include "Weapons/FortWeapon.h"
#include "Math/Vector.h"

class AFortWeaponRangedMountedCannon : public AFortWeapon
{
public:
    void ServerFireActorInCannon_Implementation(const FVector& LaunchDir);

    bool ShouldFireRiderInCannon(const FVector& LaunchDir, bool bFireRider);
};
