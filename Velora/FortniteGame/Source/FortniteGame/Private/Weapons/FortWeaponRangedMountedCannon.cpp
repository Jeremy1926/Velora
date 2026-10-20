#include "pch.h"
#include "Weapons/FortWeaponRangedMountedCannon.h"
#include "Weapons/FortMountedWeaponInterface.h"
#include "Athena/Vehicles/FortAthenaSKPushCannon.h"
#include "Pawns/FortPlayerPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation(const FVector& LaunchDir)
{
    DETOUR(AFortWeaponRangedMountedCannon, ServerFireActorInCannon_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedMountedCannon:ServerFireActorInCannon"))
        .Optional()
        .Find("call [*+0x*]", 2)
        .LogSlot(TEXT("AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortWeaponRangedMountedCannon"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortWeaponRangedMountedCannon")));

    ShouldFireRiderInCannon(LaunchDir, true);
}

bool AFortWeaponRangedMountedCannon::ShouldFireRiderInCannon(const FVector& LaunchDir, bool bFireRider)
{
    static auto FortMountedWeaponInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMountedWeaponInterface"));

    auto MountedWeaponInterface = (IFortMountedWeaponInterface*)GetInterfaceAddress(FortMountedWeaponInterface);

    if (!MountedWeaponInterface || !MountedWeaponInterface->GetHostVehicleCached())
        return false;

    auto PlayerPawn = Cast<AFortPlayerPawn>(GetInstigator());

    if (!PlayerPawn)
        return false;

    if (auto PushCannon = Cast<AFortAthenaSKPushCannon>(PlayerPawn->GetVehicle()))
    {
        if (!PushCannon->GetPawnAtSeat(1))
            return false;

        if (bFireRider && HasAuthority())
            PushCannon->ServerLaunchActorInBarrel(LaunchDir);

        return true;
    }

    return false;
}
