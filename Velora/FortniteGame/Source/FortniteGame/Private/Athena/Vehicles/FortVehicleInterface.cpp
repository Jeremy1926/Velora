#include "pch.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Scanner.h"

FTransform IFortVehicleInterface::GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const
{
    static auto GetExitSocketTransform = Scanner::Exec(TEXT("/Script/FortniteGame.FortVehicleInterface:GetExitSocketTransform")).Find("call [*+0x*]").Slot(TEXT("IFortVehicleInterface::GetExitSocketTransform"));

    FTransform Result;

    return *Scanner::Virtual<FTransform* (*)(const IFortVehicleInterface*, FTransform*, int32, int32)>(this, GetExitSocketTransform)(this, &Result, SeatIndex, SocketIndex);
}

AFortPlayerPawn* IFortVehicleInterface::GetPawnAtSeat(const int32 SeatIdx) const
{
    static auto GetPawnAtSeat = Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat")).Find("call [rax+0x*]").Slot(TEXT("IFortVehicleInterface::GetPawnAtSeat"));

    return Scanner::Virtual<AFortPlayerPawn* (*)(const IFortVehicleInterface*, int32)>(this, GetPawnAtSeat)(this, SeatIdx);
}
