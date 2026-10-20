#pragma once

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"

struct FFrame;

class AFortWeapon : public AActor
{
public:
    static UClass* StaticClass();

    void PlayWeaponFireFX(bool bPersistentFire);

    bool CheckPendingDestroyedDueToDurability();

    bool TryReloadAmmo();

    bool TryReloadAmmoFromAbility();

    void execTryReloadAmmoFromAbility(FFrame& Stack, void* const Z_Param__Result);

    bool GetPendingDestroyDueToDurabilityOrStackCount() const;

    FGuid& GetItemEntryGuid();

    float& GetLastFireTimeVerified();
};
