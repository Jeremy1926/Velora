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
    static auto VirtualCall = Scanner::Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity")).Find("call [rax+0x*]");
    static auto Slot = VirtualCall.Get() ? VirtualCall.Slot(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity")) : 0;

    if (Scanner::IsSlot(Slot))
        return Scanner::Virtual<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(this, Slot)(this, NewVel, bAddToCurrent, BoneName);

    static auto SetPhysicsLinearVelocity = Scanner::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity")).Get(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity")));

    SetPhysicsLinearVelocity(this, NewVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName)
{
    static auto VirtualCall = Scanner::Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians")).Find("call [rax+0x*]");
    static auto Slot = VirtualCall.Get() ? VirtualCall.Slot(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians")) : 0;

    if (Scanner::IsSlot(Slot))
        return Scanner::Virtual<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(this, Slot)(this, NewAngVel, bAddToCurrent, BoneName);

    static auto SetPhysicsAngularVelocityInRadians = Scanner::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians")).Get(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians")));

    SetPhysicsAngularVelocityInRadians(this, NewAngVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::IgnoreActorWhenMoving(AActor* Actor, bool bShouldIgnore)
{
    static auto IgnoreActorWhenMoving = Scanner::Function<void (*)(UPrimitiveComponent*, AActor*, bool)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:IgnoreActorWhenMoving")).Get(TEXT("UPrimitiveComponent::IgnoreActorWhenMoving")));

    IgnoreActorWhenMoving(this, Actor, bShouldIgnore);
}

void UPrimitiveComponent::GetOverlappingActors(TArray<AActor*>& OverlappingActors, TSubclassOf<AActor> ClassFilter) const
{
    static auto GetOverlappingActors = Scanner::Function<void (*)(const UPrimitiveComponent*, TArray<AActor*>&, UClass*)>(Scanner::Native(TEXT("/Script/Engine.PrimitiveComponent:GetOverlappingActors")).Get(TEXT("UPrimitiveComponent::GetOverlappingActors")));

    GetOverlappingActors(this, OverlappingActors, ClassFilter);
}
