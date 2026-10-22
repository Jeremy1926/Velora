#pragma once

#include "Components/ActorComponent.h"

struct FFrame;
class AFortAthenaVehicle;
enum class EVehicleSeats : uint8;

class UFortControllerComponent_SpawnedVehicle : public UActorComponent
{
public:
    static UClass* StaticClass();

    void execExitSpawnedVehicle(FFrame& Stack, void* const Z_Param__Result);

    AFortAthenaVehicle* SpawnVehicleAndEnter(UClass* VehicleClass, bool bSetPawnAsVehicleOwner, EVehicleSeats SeatToEnter, bool bIgnoreDBNOCheck);

    void ExitSpawnedVehicle();

    AFortAthenaVehicle*& GetSpawnedVehicle();
};
