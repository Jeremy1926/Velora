#pragma once

#include "GameFramework/Actor.h"
#include "Misc/Guid.h"
#include "UObject/WeakObjectPtr.h"
#include "GameplayAbilitySpec.h"

struct FFrame;
class UFortGameplayAbility;
class UFortItemDefinition;
class UFortWeaponItemDefinition;
class UFortWeaponModItemDefinition;
class UFortWorldItem;
struct FFortWeaponModSlot;
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

    static TArray<FWeakObjectPtr>& GetPendingUnequipRemovals();

    static void FlushPendingUnequipRemovals();

    void RemoveGrantedAbilities();

    bool GetPendingDestroyDueToDurabilityOrStackCount() const;

    FGuid& GetItemEntryGuid();

    void InitWeapon(UFortWorldItem* Item);

    UFortWorldItem* GetInventoryItem() const;

    const TArray<FFortWeaponModSlot>& GetWeaponModSlots() const;

    bool CanAddWeaponMod(const UFortWeaponModItemDefinition* WeaponMod) const;

    bool AddWeaponModSlot(const FFortWeaponModSlot& WeaponModSlot);

    bool RemoveWeaponModSlot(int32 SlotIndex);

    void UpdateCurrentWeaponModSlotsOnWorldItem();

    float& GetLastFireTimeVerified();

    UFortGameplayAbility* GetActiveAbility() const;

    FGameplayAbilitySpecHandle GetPrimaryAbilitySpecHandle() const;
};
