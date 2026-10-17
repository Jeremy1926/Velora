#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Math/Rotator.h"

class AController : public AActor
{
public:
    APawn* GetPawn() const;

    AActor* GetViewTarget() const;

    FRotator GetControlRotation() const;

    void GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const;
};
