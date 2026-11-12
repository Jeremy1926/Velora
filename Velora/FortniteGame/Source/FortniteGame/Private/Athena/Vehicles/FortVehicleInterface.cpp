#include "pch.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Scanner.h"

FTransform IFortVehicleInterface::GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const
{
    static auto GetExitSocketTransform = Anchor::VirtualFunction<TGameValue<FTransform>* (*)(const IFortVehicleInterface*, TGameValue<FTransform>*, int32, int32)>(TEXT("IFortVehicleInterface::GetExitSocketTransform"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Find("call [*+0x*]")
        .Get();

    TGameValue<FTransform> Result;

    return GetExitSocketTransform(this, &Result, SeatIndex, SocketIndex)->Get();
}

AFortPlayerPawn* IFortVehicleInterface::GetPawnAtSeat(const int32 SeatIdx) const
{
    static auto GetPawnAtSeat = Anchor::VirtualFunction<AFortPlayerPawn* (*)(const IFortVehicleInterface*, int32)>(TEXT("IFortVehicleInterface::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *; add r*, r*")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("call [rax+0x*]")
        .Get();

    return GetPawnAtSeat(this, SeatIdx);
}

UFortVehicleSeatWeaponComponent* IFortVehicleInterface::GetSeatWeaponComponent(const int32 SeatIndex) const
{
    static auto GetSeatWeaponComponent = Anchor::VirtualFunction<UFortVehicleSeatWeaponComponent* (*)(const IFortVehicleInterface*, int32)>(TEXT("IFortVehicleInterface::GetSeatWeaponComponent"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetSeatWeaponComponent"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetSeatWeaponComponent"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetSeatWeaponComponent"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetSeatWeaponComponent"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetSeatWeaponComponent"))
        .Find("call [*+0x*]")
        .Get();

    return GetSeatWeaponComponent(this, SeatIndex);
}

const AActor* IFortVehicleInterface::GetVehicleActorPtrConst() const
{
    static auto GetVehicleActorPtrConst = Anchor::VirtualFunction<const AActor* (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::GetVehicleActorPtrConst"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleActorPtrConst"))
        .Find("call [*+0x*]")
        .Get();

    return GetVehicleActorPtrConst(this);
}

AFortPlayerControllerZone* IFortVehicleInterface::GetVehicleController(AFortPlayerPawn* PlayerPawn) const
{
    static auto GetVehicleController = Anchor::VirtualFunction<AFortPlayerControllerZone* (*)(const IFortVehicleInterface*, AFortPlayerPawn*)>(TEXT("IFortVehicleInterface::GetVehicleController"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleController"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleController"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleController"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleController"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Optional()
        .Get();

    return GetVehicleController ? GetVehicleController(this, PlayerPawn) : nullptr;
}

void IFortVehicleInterface::SetForcedToAlwaysSpawn(const bool bSetTo)
{
    static auto SetForcedToAlwaysSpawn = Anchor::VirtualFunction<void (*)(IFortVehicleInterface*, bool)>(TEXT("IFortVehicleInterface::SetForcedToAlwaysSpawn"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:SetForcedToAlwaysSpawn"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:SetForcedToAlwaysSpawn"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:SetForcedToAlwaysSpawn"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:SetForcedToAlwaysSpawn"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:SetForcedToAlwaysSpawn"))
        .Find("call [*+0x*]")
        .Get();

    SetForcedToAlwaysSpawn(this, bSetTo);
}

const UFortVehicleConfigs* IFortVehicleInterface::GetVehicleConfigs() const
{
    static auto GetVehicleConfigs = Anchor::VirtualFunction<const UFortVehicleConfigs* (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::GetVehicleConfigs"))
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call [rax+0x*]")
        .Get();

    return GetVehicleConfigs(this);
}

USkeletalMeshComponent* IFortVehicleInterface::GetSkeletalMeshComponent() const
{
    static auto GetSkeletalMeshComponent = Anchor::VirtualFunction<USkeletalMeshComponent* (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::GetSkeletalMeshComponent"))
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call [*+0x*]", 3)
        .Is("call [*+0x*]; mov *, rax; test rax, rax")
        .Or()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call [*+0x*]; mov rbx, rax; test rax, rax")
        .Or()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call 0x*; test rax, rax; jnz *")
        .Find("jnz 0x*")
        .Target()
        .Find("call [*+0x*]; mov rbx, rax; test rax, rax")
        .Get();

    return GetSkeletalMeshComponent(this);
}

bool IFortVehicleInterface::ContainsPlayers() const
{
    static auto ContainsPlayers = Anchor::VirtualFunction<bool (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::ContainsPlayers"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:ContainsPlayers"))
        .Find("call [rax+0x*]")
        .Get();

    return ContainsPlayers(this);
}
