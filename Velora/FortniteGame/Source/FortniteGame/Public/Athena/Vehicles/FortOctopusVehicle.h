#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Math/Vector.h"

class AFortOctopusVehicle : public AFortAthenaVehicle
{
public:
    void ServerUpdateTowhook_Implementation(const TGameValue<FVector>& GameNetTowhookAimDir);
};
