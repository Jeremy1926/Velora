#pragma once

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"

struct FFrame;
class UFortItemDefinition;
class UFortWeaponItemDefinition;
enum class EFortWeaponType : uint8;

enum class EFortWeaponAbilityRemovalReason : uint8
{
    RemovedFromInventory,
    EndPlay,
    Unequipped,
};

enum class EFortWeaponAbilityRemovalPolicy : uint8
{
    GameDefault,
    Remove,
    Persist,
};

class AFortWeapon : public AActor
{
public:
    static UClass* StaticClass();

    static EFortWeaponType FindWeaponTypeForStats(const UFortItemDefinition* ItemDefinition);

    UFortWeaponItemDefinition* GetWeaponData() const;

    void PlayWeaponFireFX(bool bPersistentFire);

    bool CheckPendingDestroyedDueToDurability();

    bool TryReloadAmmo();

    bool TryReloadAmmoFromAbility();

    void execTryReloadAmmoFromAbility(FFrame& Stack, void* const Z_Param__Result);

    void ModAmmo(int32 AmountToUse);

    void OnUnEquip();

    void AttemptRemoveGrantedAbilities(EFortWeaponAbilityRemovalReason Reason);

    void RemoveGrantedAbilities();

    bool GetPendingDestroyDueToDurabilityOrStackCount() const;

    FGuid& GetItemEntryGuid();

    float& GetLastFireTimeVerified();
};
