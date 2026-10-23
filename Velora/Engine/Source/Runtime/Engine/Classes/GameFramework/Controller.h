#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Math/Rotator.h"
#include "UObject/ScriptDelegates.h"

class APlayerState;

class AController : public AActor
{
public:
    static UClass* StaticClass();

    APawn* GetPawn() const;

    APlayerState* GetPlayerState() const;

    FMulticastScriptDelegate& GetOnPossessedPawnChanged();

    AActor* GetViewTarget() const;

    FRotator GetControlRotation() const;

    void SetControlRotation(const FRotator& NewRotation);

    FName GetStateName();

    void SetStateName(FName InStateName);

    void GetPlayerViewPoint(FVector& Location, FRotator& Rotation) const;
};
