#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"

class AActor;
class UFortControllerComponent_SpawnedVehicle;

class AFortNevadaVehicle : public AFortAthenaVehicle
{
public:
    static UClass* StaticClass();

    bool AttachActorToTractorBeam(AActor* Actor);

    void CleanupTractorBeamAttachments();

    static UFortControllerComponent_SpawnedVehicle* GetSpawnedVehicleComponent(AActor* Actor);

    AActor* GetTractorBeamAttachedActor() const;

    UClass* GetAttachedPawnHiddenVehicle() const;
};
