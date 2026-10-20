#include "pch.h"
#include "Weapons/FortProjectileMovementComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortProjectileMovementComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortProjectileMovementComponent"));

    return Class;
}

void UFortProjectileMovementComponent::SetVelocity(const FVector& InReplicatedVelocity)
{
    static auto SetVelocity = Scanner::Function<void (*)(UFortProjectileMovementComponent*, const FVector&)>(Scanner::Native(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity")).Get(TEXT("UFortProjectileMovementComponent::SetVelocity")));

    SetVelocity(this, InReplicatedVelocity);
}
