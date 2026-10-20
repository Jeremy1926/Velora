#include "pch.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Scanner.h"

FTransform IFortVehicleInterface::GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const
{
    static auto GetExitSocketTransform = Anchor::Slot(TEXT("IFortVehicleInterface::GetExitSocketTransform"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform"))
        .Find("call [*+0x*]")
        .Get();

    FTransform Result;

    return *Scanner::Virtual<FTransform* (*)(const IFortVehicleInterface*, FTransform*, int32, int32)>(this, GetExitSocketTransform)(this, &Result, SeatIndex, SocketIndex);
}

AFortPlayerPawn* IFortVehicleInterface::GetPawnAtSeat(const int32 SeatIdx) const
{
    static auto GetPawnAtSeat = Anchor::Slot(TEXT("IFortVehicleInterface::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("call [rax+0x*]")
        .Get();

    return Scanner::Virtual<AFortPlayerPawn* (*)(const IFortVehicleInterface*, int32)>(this, GetPawnAtSeat)(this, SeatIdx);
}

const AActor* IFortVehicleInterface::GetVehicleActorPtrConst() const
{
    static auto GetVehicleActorPtrConst = Anchor::Slot(TEXT("IFortVehicleInterface::GetVehicleActorPtrConst"))
        .Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetVehicleActorPtrConst"))
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<const AActor* (*)(const IFortVehicleInterface*)>(this, GetVehicleActorPtrConst)(this);
}
