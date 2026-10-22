#pragma once

#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"
#include "Containers/Array.h"
#include "Templates/SubclassOf.h"

class AActor;
struct FBodyInstance;

class UPrimitiveComponent : public USceneComponent
{
public:
    static UClass* StaticClass();

    void SetPhysicsLinearVelocity(FVector NewVel, bool bAddToCurrent, FName BoneName);

    void SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName);

    void IgnoreActorWhenMoving(AActor* Actor, bool bShouldIgnore);

    void GetOverlappingActors(TArray<AActor*>& OverlappingActors, TSubclassOf<AActor> ClassFilter) const;

    FBodyInstance* GetBodyInstance(FName BoneName, bool bGetWelded) const;

    void SetCollisionEnabled(ECollisionEnabled::Type NewType);
};
