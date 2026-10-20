#include "pch.h"
#include "Weapons/FortMountedWeaponInterface.h"
#include "Pawns/FortPlayerPawn.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FMountedWeaponInfoRepped::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.MountedWeaponInfoRepped"));

    return Struct;
}

AActor*& FMountedWeaponInfoRepped::GetHostVehicleCachedActor()
{
    static auto HostVehicleCachedActor = StaticStruct()->FindPropertyByName(FName(TEXT("HostVehicleCachedActor")));

    return *HostVehicleCachedActor->ContainerPtrToValuePtr<AActor*>(this);
}

int32& FMountedWeaponInfoRepped::GetHostVehicleSeatIndexCached()
{
    static auto HostVehicleSeatIndexCached = StaticStruct()->FindPropertyByName(FName(TEXT("HostVehicleSeatIndexCached")));

    return *HostVehicleSeatIndexCached->ContainerPtrToValuePtr<int32>(this);
}

AActor* IFortMountedWeaponInterface::GetHostVehicleCached()
{
    static auto GetHostVehicleCached = Scanner::Exec(TEXT("/Script/FortniteGame.FortMountedWeaponInterface:GetHostVehicleCached")).Find("call [*+0x*]").Slot(TEXT("IFortMountedWeaponInterface::GetHostVehicleCached"));

    return Scanner::Virtual<AActor* (*)(IFortMountedWeaponInterface*)>(this, GetHostVehicleCached)(this);
}

FMountedWeaponInfoRepped& IFortMountedWeaponInterface::GetMountedWeaponInfoRepped()
{
    static auto FinishMountedWeaponInitialization = Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped")).Find("jmp 0x*").Target()).Call();
    static auto GetMountedWeaponInfoRepped = FinishMountedWeaponInitialization.Find("call [rax+0x*]").Slot(TEXT("IFortMountedWeaponInterface::GetMountedWeaponInfoRepped"));

    return *Scanner::Virtual<FMountedWeaponInfoRepped* (*)(IFortMountedWeaponInterface*)>(this, GetMountedWeaponInfoRepped)(this);
}

AFortPlayerPawn* IFortMountedWeaponInterface::GetInstigatorPawn()
{
    static auto ReattachWeaponIfNeeded = Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped")).Find("jmp 0x*").Target()).Find("mov [rsp+0x20], *; call 0x*").Call();
    static auto GetInstigatorPawn = ReattachWeaponIfNeeded.Find("call [rax+0x*]", 2).Slot(TEXT("IFortMountedWeaponInterface::GetInstigatorPawn"));

    return Scanner::Virtual<AFortPlayerPawn* (*)(IFortMountedWeaponInterface*)>(this, GetInstigatorPawn)(this);
}

void IFortMountedWeaponInterface::SetupCachedHostVehicleRepped(AFortWeaponRanged* Weapon)
{
    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto& MountedWeaponInfoRepped = GetMountedWeaponInfoRepped();

    if (MountedWeaponInfoRepped.GetHostVehicleCachedActor())
        return;

    auto PlayerPawn = GetInstigatorPawn();

    if (!PlayerPawn)
        return;

    auto Vehicle = PlayerPawn->GetVehicleStateLocal().GetVehicle();

    if (!Vehicle || !Vehicle->GetInterfaceAddress(FortVehicleInterface))
        return;

    MountedWeaponInfoRepped.GetHostVehicleCachedActor() = Vehicle;
    MountedWeaponInfoRepped.GetHostVehicleSeatIndexCached() = PlayerPawn->GetVehicleStateLocal().GetSeatIndex();

    FinishMountedWeaponInitialization(Weapon);
}

void IFortMountedWeaponInterface::FinishMountedWeaponInitialization(AFortWeaponRanged* Weapon)
{
    static auto FinishMountedWeaponInitialization = Scanner::Function<void (*)(IFortMountedWeaponInterface*, AFortWeaponRanged*)>(Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped")).Find("jmp 0x*").Target()).Call().Get(TEXT("IFortMountedWeaponInterface::FinishMountedWeaponInitialization")));

    FinishMountedWeaponInitialization(this, Weapon);
}
