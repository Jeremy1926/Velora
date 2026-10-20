#pragma once

#include "HAL/Platform.h"

class AActor;
class AFortPlayerPawn;
class AFortWeaponRanged;
class UScriptStruct;

struct FMountedWeaponInfoRepped
{
    static UScriptStruct* StaticStruct();

    AActor*& GetHostVehicleCachedActor();

    int32& GetHostVehicleSeatIndexCached();
};

class IFortMountedWeaponInterface
{
public:
    AActor* GetHostVehicleCached();

    FMountedWeaponInfoRepped& GetMountedWeaponInfoRepped();

    AFortPlayerPawn* GetInstigatorPawn();

    void SetupCachedHostVehicleRepped(AFortWeaponRanged* Weapon);

    void FinishMountedWeaponInitialization(AFortWeaponRanged* Weapon);
};
