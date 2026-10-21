#pragma once

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"

struct FFrame;
class UFortItemDefinition;
enum class EFortWeaponType : uint8;

class AFortWeapon : public AActor
{
public:
    static UClass* StaticClass();

    static EFortWeaponType FindWeaponTypeForStats(const UFortItemDefinition* ItemDefinition);

    void PlayWeaponFireFX(bool bPersistentFire);

    bool CheckPendingDestroyedDueToDurability();

    bool TryReloadAmmo();

    bool TryReloadAmmoFromAbility();

    void execTryReloadAmmoFromAbility(FFrame& Stack, void* const Z_Param__Result);

    void ModAmmo(int32 AmountToUse);

    bool GetPendingDestroyDueToDurabilityOrStackCount() const;

    FGuid& GetItemEntryGuid();

    float& GetLastFireTimeVerified();
};
