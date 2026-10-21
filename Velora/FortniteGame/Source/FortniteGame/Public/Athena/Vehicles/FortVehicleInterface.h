#pragma once

#include "Math/TransformVectorized.h"

class AActor;
class AFortPlayerPawn;
class UFortVehicleSeatWeaponComponent;

class IFortVehicleInterface
{
public:
    FTransform GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const;

    AFortPlayerPawn* GetPawnAtSeat(const int32 SeatIdx) const;

    UFortVehicleSeatWeaponComponent* GetSeatWeaponComponent(const int32 SeatIndex) const;

    const AActor* GetVehicleActorPtrConst() const;
};
