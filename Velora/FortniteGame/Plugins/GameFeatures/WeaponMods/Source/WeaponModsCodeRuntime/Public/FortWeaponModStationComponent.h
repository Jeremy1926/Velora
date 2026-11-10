#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

class AController;
class AFortPickup;
class AFortWeapon;
class UFortItemDefinition;
class UFortWeaponModItemDefinition;
struct FScalableFloat;

class AFortWeaponModStationBase : public AActor
{
public:
    bool GetModCostOverride(float& OutCost);
};

class UFortWeaponModStationComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerPurchaseWeaponModForWeapon_Implementation(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon);

    void ServerPurchaseRemoveMod_Implementation(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon);

    bool CanPlayerAffordModForWeapon(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon, AController* Controller);

    bool CanPlayerAffordRemoveModFromWeapon(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon, AController* Controller);

    float GetCostForWeaponMod(UFortWeaponModItemDefinition* WeaponMod);

    UFortItemDefinition* GetTransactionCurrency();

    AFortWeaponModStationBase* GetCurrentInteractingWeaponModStation() const;

    const FScalableFloat& GetRemoveModCost() const;

    UFortWeaponModItemDefinition* GetDefaultIronSightMod() const;

    UFortWeaponModItemDefinition* GetDefaultMagazineMod() const;
};

class UFortPickupInteractOverrideComponent_WeaponMods : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerApplyWeaponModFromPickup_Implementation(AFortPickup* Pickup, UFortWeaponModItemDefinition* WeaponModDef, AFortWeapon* Weapon);
};
