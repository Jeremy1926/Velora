#pragma once

#include "GameFramework/Actor.h"
#include "UObject/ScriptDelegates.h"

class AFortVehicleManager : public AActor
{
public:
    static UClass* StaticClass();

    static bool GetVehicleManager(const UObject* WorldContextObject, AFortVehicleManager*& VehicleManager);

    void UpdateVehicleSpawns();

    FMulticastScriptDelegate* GetOnVehicleSpawnsUpdated();
};
