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

static auto ServerFireActorInCannon = []
{
    auto ServerFireActorInCannon = Scanner::Exec(TEXT("/Script/FortniteGame.FortWeaponRangedMountedCannon:ServerFireActorInCannon"));
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeaponRangedMountedCannon"));
    auto Slot = ServerFireActorInCannon.Get() ? ServerFireActorInCannon.Find("call [*+0x*]", 2).Slot(TEXT("AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation")) : 0;

    return Slot && Scanner::At(Scanner::Virtual<const void*>(DefaultObject, Slot)).Is("ret*") ? DetourVTable(TEXT("AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation"), *(void***)DefaultObject, Slot, &AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation) : nullptr;
}();

void AFortWeaponRangedMountedCannon::ServerFireActorInCannon_Implementation(const FVector& LaunchDir)
{
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
