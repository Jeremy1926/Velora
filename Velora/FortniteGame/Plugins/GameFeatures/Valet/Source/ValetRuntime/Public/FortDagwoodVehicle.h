#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"

class AFortDagwoodVehicle : public AFortAthenaVehicle
{
public:
    static UClass* StaticClass();

    void MakeInoperable();

    bool IsInoperable() const;

    void AddSpawningFuel();

    void OnRep_IsInoperable();
};
