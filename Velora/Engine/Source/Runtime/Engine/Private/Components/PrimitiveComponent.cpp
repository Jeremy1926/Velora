#include "pch.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UPrimitiveComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.PrimitiveComponent"));

    return Class;
}

void UPrimitiveComponent::SetPhysicsLinearVelocity(FVector NewVel, bool bAddToCurrent, FName BoneName)
{
    static auto SetPhysicsLinearVelocity = Scanner::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity")).Get(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity")));

    SetPhysicsLinearVelocity(this, NewVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName)
{
    static auto SetPhysicsAngularVelocityInRadians = Scanner::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians")).Get(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians")));

    SetPhysicsAngularVelocityInRadians(this, NewAngVel, bAddToCurrent, BoneName);
}
