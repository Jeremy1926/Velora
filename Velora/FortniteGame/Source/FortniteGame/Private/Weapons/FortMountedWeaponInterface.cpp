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
    static auto GetHostVehicleCached = Anchor::VirtualFunction<AActor* (*)(IFortMountedWeaponInterface*)>(TEXT("IFortMountedWeaponInterface::GetHostVehicleCached"))
        .Exec(TEXT("/Script/FortniteGame.FortMountedWeaponInterface:GetHostVehicleCached"))
        .Find("call [*+0x*]")
        .Get();

    return GetHostVehicleCached(this);
}

FMountedWeaponInfoRepped& IFortMountedWeaponInterface::GetMountedWeaponInfoRepped()
{
    static auto GetMountedWeaponInfoRepped = Anchor::VirtualFunction<FMountedWeaponInfoRepped* (*)(IFortMountedWeaponInterface*)>(TEXT("IFortMountedWeaponInterface::GetMountedWeaponInfoRepped"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("jmp 0x*")
        .Target()
        .Call()
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Get();

    return *GetMountedWeaponInfoRepped(this);
}

AFortPlayerPawn* IFortMountedWeaponInterface::GetInstigatorPawn()
{
    static auto GetInstigatorPawn = Anchor::VirtualFunction<AFortPlayerPawn* (*)(IFortMountedWeaponInterface*)>(TEXT("IFortMountedWeaponInterface::GetInstigatorPawn"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov [rsp+0x20], *; call 0x*")
        .Call()
        .Find("call [rax+0x*]", 2)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov [rsp+0x20], *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 2)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("cmp qword ptr [rax+0x*], 0x0; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("mov [rsp+0x20], *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 2)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov [rsp+0x20], *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("cmp byte ptr [rax+0x*], 0x0; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("test rax, rax; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("mov [rsp+0x20], *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 2)
        .Get();

    return GetInstigatorPawn(this);
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
    static auto FinishMountedWeaponInitialization = Anchor::Function<void (*)(IFortMountedWeaponInterface*, AFortWeaponRanged*)>(TEXT("IFortMountedWeaponInterface::FinishMountedWeaponInitialization"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("jmp 0x*")
        .Target()
        .Call()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRangedForVehicle:OnRep_MountedWeaponInfoRepped"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Get();

    FinishMountedWeaponInitialization(this, Weapon);
}
