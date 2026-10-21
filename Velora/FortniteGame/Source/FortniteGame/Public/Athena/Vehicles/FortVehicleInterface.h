#pragma once

#include "Math/TransformVectorized.h"

class AActor;
class AFortPlayerPawn;
class UFortVehicleConfigs;
class UFortVehicleSeatWeaponComponent;
class USkeletalMeshComponent;

class IFortVehicleInterface
{
public:
    FTransform GetExitSocketTransform(const int32 SeatIndex, const int32 SocketIndex) const;

    AFortPlayerPawn* GetPawnAtSeat(const int32 SeatIdx) const;

    UFortVehicleSeatWeaponComponent* GetSeatWeaponComponent(const int32 SeatIndex) const;

    const AActor* GetVehicleActorPtrConst() const;

    void SetForcedToAlwaysSpawn(const bool bSetTo);

    const UFortVehicleConfigs* GetVehicleConfigs() const;

    USkeletalMeshComponent* GetSkeletalMeshComponent() const;
};
