#pragma once

#include "Components/SceneComponent.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AActor;

class UPrimitiveComponent : public USceneComponent
{
public:
    static UClass* StaticClass();

    void SetPhysicsLinearVelocity(FVector NewVel, bool bAddToCurrent, FName BoneName);

    void SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName);

    void IgnoreActorWhenMoving(AActor* Actor, bool bShouldIgnore);
};
