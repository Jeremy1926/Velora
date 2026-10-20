#pragma once

#include "Math/TransformVectorized.h"

class IFortVehicleInterface
{
public:
    FTransform GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const;
};
