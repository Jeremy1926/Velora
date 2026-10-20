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

    static auto SetPhysicsLinearVelocity = Anchor::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity"))
        .Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Get();

    SetPhysicsLinearVelocity(this, NewVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName)
{
    static auto VirtualCall = Scanner::Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians")).Find("call [rax+0x*]");
    static auto Slot = VirtualCall.Get() ? VirtualCall.Slot(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians")) : 0;

    if (Scanner::IsSlot(Slot))
        return Scanner::Virtual<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(this, Slot)(this, NewAngVel, bAddToCurrent, BoneName);

    static auto SetPhysicsAngularVelocityInRadians = Anchor::Function<void (*)(UPrimitiveComponent*, FVector, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians"))
        .Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians"))
        .Get();

    SetPhysicsAngularVelocityInRadians(this, NewAngVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::IgnoreActorWhenMoving(AActor* Actor, bool bShouldIgnore)
{
    static auto IgnoreActorWhenMoving = Anchor::Function<void (*)(UPrimitiveComponent*, AActor*, bool)>(TEXT("UPrimitiveComponent::IgnoreActorWhenMoving"))
        .Native(TEXT("/Script/Engine.PrimitiveComponent:IgnoreActorWhenMoving"))
        .Get();

    IgnoreActorWhenMoving(this, Actor, bShouldIgnore);
}

void UPrimitiveComponent::GetOverlappingActors(TArray<AActor*>& OverlappingActors, TSubclassOf<AActor> ClassFilter) const
{
    static auto GetOverlappingActors = Anchor::Function<void (*)(const UPrimitiveComponent*, TArray<AActor*>&, UClass*)>(TEXT("UPrimitiveComponent::GetOverlappingActors"))
        .Native(TEXT("/Script/Engine.PrimitiveComponent:GetOverlappingActors"))
        .Get();

    GetOverlappingActors(this, OverlappingActors, ClassFilter);
}
