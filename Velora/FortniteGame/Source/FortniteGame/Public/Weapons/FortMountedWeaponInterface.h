#pragma once

#include "HAL/Platform.h"
#include "UObject/ScriptInterface.h"

class AActor;
class AFortPlayerPawn;
class AFortWeaponRanged;
class IFortVehicleInterface;
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

    TScriptInterface<IFortVehicleInterface> BP_GetHostVehicle();

    FMountedWeaponInfoRepped& GetMountedWeaponInfoRepped();

    AFortPlayerPawn* GetInstigatorPawn();

    void SetupCachedHostVehicleRepped(AFortWeaponRanged* Weapon);

    void FinishMountedWeaponInitialization(AFortWeaponRanged* Weapon);
};
