#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"

class AActor;
class ABuildingActor;
class AFortAthenaVehicleSpawner;
class UFortVehicleItemDefinition;
struct FFrame;
struct FVehiclePropReplacementData;
enum class ESpawnActorCollisionHandlingMethod : uint8;

class UFortDagwoodVehicleManager : public UActorComponent
{
public:
    static UClass* StaticClass();

    void OnVehicleSpawnsUpdated(UFortVehicleItemDefinition* VehicleItemDef, TArray<AActor*>& SpawnedVehicles, int32 NewVehicleCount);

    void UpdatePropReplacement();

    void execUpdatePropReplacement(FFrame& Stack, void* const Z_Param__Result);

    AFortAthenaVehicleSpawner* SwapOutProp(ABuildingActor* BuildingProp, UClass* VehicleSpawnerClass, const FVehiclePropReplacementData& PropReplacementData, ESpawnActorCollisionHandlingMethod SpawnActorCollisionHandlingMethod);
};
