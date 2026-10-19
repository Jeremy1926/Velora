#pragma once

#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

struct FReplicatedAthenaVehiclePhysicsState
{
    FVector Translation;
    FQuat Rotation;
    FVector LinearVelocity;
    FVector AngularVelocity;
};

struct FReplicatedAthenaVehicleInputState;

class AFortAthenaVehicle : public AActor
{
public:
    static UClass* StaticClass();

    static void InitializeServerImplementations();

    static const TCHAR* GetServerFunctionPath(const TCHAR* FunctionName);

    void ServerUpdatePhysicsParams_Implementation(const FReplicatedAthenaVehiclePhysicsState& InState);

    void ServerUpdateInputParams_Implementation(const FReplicatedAthenaVehicleInputState& InState);

    void WakeUp();
};
