#include "pch.h"
#include "Weapons/FortMountedWeaponInterface.h"
#include "Scanner.h"

AActor* IFortMountedWeaponInterface::GetHostVehicleCached()
{
    static auto GetHostVehicleCached = Scanner::Exec(TEXT("/Script/FortniteGame.FortMountedWeaponInterface:GetHostVehicleCached")).Find("call [*+0x*]").Slot(TEXT("IFortMountedWeaponInterface::GetHostVehicleCached"));

    return Scanner::Virtual<AActor* (*)(IFortMountedWeaponInterface*)>(this, GetHostVehicleCached)(this);
}
