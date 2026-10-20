#pragma once

#include "Math/TransformVectorized.h"

class AFortPlayerPawn;

class IFortVehicleInterface
{
public:
    FTransform GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const;

    AFortPlayerPawn* GetPawnAtSeat(const int32 SeatIdx) const;
};
