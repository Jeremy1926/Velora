#pragma once

#include "Components/ActorComponent.h"
#include "Math/Rotator.h"

class UFortControllerComponent_Aircraft : public UActorComponent
{
public:
    static UClass* StaticClass();

    bool IsInAircraft() const;

    bool& GetIsAttemptingAircraftJump();

    void ServerAttemptAircraftJump_Implementation(FRotator ClientRotation);
};
