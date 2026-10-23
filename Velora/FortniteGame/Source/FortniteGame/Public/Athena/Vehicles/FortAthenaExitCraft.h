#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Delegates/Delegate.h"

class AFortAthenaExitCraft : public AFortAthenaVehicle
{
public:
    static TMulticastDelegate<void, AFortAthenaExitCraft*, float>& GetOnExitVehicleCullDistChange();
};
