#pragma once

#include "Weapons/FortWeapon.h"
#include "Math/Vector.h"

struct FHitResult;

class AFortWeaponRanged : public AFortWeapon
{
public:
    static UClass* StaticClass();

    void ServerNotifyPawnHit_Implementation(const FHitResult& Hit, const TGameValue<FVector>& ProjectileOriginPosition, float ProjectileStartTimestamp, const TArray<uint8>& Context);
};
