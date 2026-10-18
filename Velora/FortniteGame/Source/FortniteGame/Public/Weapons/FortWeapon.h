#pragma once

#include "GameFramework/Actor.h"

class AFortWeapon : public AActor
{
public:
    static UClass* StaticClass();

    void PlayWeaponFireFX(bool bPersistentFire);

    bool CheckPendingDestroyedDueToDurability();

    bool GetPendingDestroyDueToDurabilityOrStackCount() const;
};
