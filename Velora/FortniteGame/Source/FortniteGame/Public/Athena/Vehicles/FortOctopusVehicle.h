#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Engine/NetSerialization.h"

class AFortOctopusVehicle : public AFortAthenaVehicle
{
public:
    void ServerUpdateTowhook_Implementation(FVector_NetQuantizeNormal InNetTowhookAimDir);
};
