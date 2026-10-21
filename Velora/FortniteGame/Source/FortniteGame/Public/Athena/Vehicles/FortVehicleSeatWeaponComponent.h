#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "UObject/WeakObjectPtr.h"

class AFortPawn;
class AFortPlayerPawn;
class AFortWeapon;
class IFortVehicleInterface;
class UFortVehicleAimingWeaponComp;
class UFortWeaponItemDefinition;
class UFortWorldItem;
class UScriptStruct;

struct FWeaponSeatDefinition
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    int32 GetSeatIndex();

    UFortWeaponItemDefinition* GetVehicleWeapon();

    UFortWeaponItemDefinition* GetVehicleWeaponOverride();

    UFortWeaponItemDefinition*& GetLastEquippedVehicleWeapon();

    TWeakObjectPtr<UFortWorldItem>& GetVehicleGrantedWeaponItem();

    TWeakObjectPtr<AFortWeapon>& GetVehicleGrantedWeapon();
};

template<>
class TArray<FWeaponSeatDefinition, FDefaultAllocator> : public TRuntimeSizedArray<FWeaponSeatDefinition>
{
};

struct FVehicleWeapon_RetainedData
{
    int32 AmmoInClip;
    float LastFireTime;
    bool bHasPrevious;
};

class UFortVehicleSeatWeaponComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void OnPawnEnterSeat(IFortVehicleInterface* Vehicle, int32 SeatIdx, bool bSwitchingSeats);

    void OnPawnExitSeat(IFortVehicleInterface* Vehicle, AFortPlayerPawn* PlayerPawn, int32 SeatIdx);

    void EquipVehicleWeapon(AFortPawn* FortPawn, FWeaponSeatDefinition& WeaponSeatDefinition, int32 ItemLevel);

    TArray<FWeaponSeatDefinition>& GetWeaponSeatDefinitions();

    bool GetSaveAndRestoreWeaponData() const;

    FVehicleWeapon_RetainedData* GetCachedWeaponState();
};
