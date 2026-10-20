#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Math/Rotator.h"

class APlayerState;

class AController : public AActor
{
public:
    APawn* GetPawn() const;

    APlayerState* GetPlayerState() const;

    AActor* GetViewTarget() const;

    FRotator GetControlRotation() const;

    void SetControlRotation(const FRotator& NewRotation);

    FName& GetStateName();

    void GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const;
};
