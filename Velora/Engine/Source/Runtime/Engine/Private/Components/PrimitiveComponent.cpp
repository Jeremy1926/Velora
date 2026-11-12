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
    static auto VirtualSetPhysicsLinearVelocity = Anchor::VirtualFunction<void (*)(UPrimitiveComponent*, TGameValue<FVector>, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity"))
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Has("bt rax, 0x8")
        .Find("setnz *; *; add r*, r*")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Find("call [rax+0x*]")
        .Optional()
        .Get();

    if (VirtualSetPhysicsLinearVelocity)
        return VirtualSetPhysicsLinearVelocity(this, NewVel, bAddToCurrent, BoneName);

    static auto SetPhysicsLinearVelocity = Anchor::Function<void (*)(UPrimitiveComponent*, TGameValue<FVector>, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsLinearVelocity"))
        .Native(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsLinearVelocity"))
        .Get();

    SetPhysicsLinearVelocity(this, NewVel, bAddToCurrent, BoneName);
}

void UPrimitiveComponent::SetPhysicsAngularVelocityInRadians(FVector NewAngVel, bool bAddToCurrent, FName BoneName)
{
    static auto VirtualSetPhysicsAngularVelocityInRadians = Anchor::VirtualFunction<void (*)(UPrimitiveComponent*, TGameValue<FVector>, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians"))
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians"))
        .Has("bt rax, 0x8")
        .Find("setnz *; *; add r*, r*")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetPhysicsAngularVelocityInRadians"))
        .Find("call [rax+0x*]")
        .Optional()
        .Get();

    if (VirtualSetPhysicsAngularVelocityInRadians)
        return VirtualSetPhysicsAngularVelocityInRadians(this, NewAngVel, bAddToCurrent, BoneName);

    static auto SetPhysicsAngularVelocityInRadians = Anchor::Function<void (*)(UPrimitiveComponent*, TGameValue<FVector>, bool, FName)>(TEXT("UPrimitiveComponent::SetPhysicsAngularVelocityInRadians"))
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

    static auto bClassByAddress = Anchor::Exists()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:GetOverlappingActors"))
        .Find("add [*+0x20], *")
        .Find("lea r8, [*]; mov rdx, *; mov rcx, *; call 0x*")
        .Get();

    if (bClassByAddress)
        return ((void (*)(const UPrimitiveComponent*, TArray<AActor*>&, const TSubclassOf<AActor>*))GetOverlappingActors)(this, OverlappingActors, &ClassFilter);

    GetOverlappingActors(this, OverlappingActors, ClassFilter);
}

FBodyInstance* UPrimitiveComponent::GetBodyInstance(FName BoneName, bool bGetWelded) const
{
    static auto GetBodyInstance = Anchor::VirtualFunction<FBodyInstance* (*)(const UPrimitiveComponent*, FName, bool, int32)>(TEXT("UPrimitiveComponent::GetBodyInstance"))
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("or r9d, 0xffffffff; mov r8b, 0x1; *; mov r10, [rcx+0x*]")
        .Find("mov r10, [rcx+0x*]")
        .Or()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("mov r9d, e*; mov r8b, 0x1; xor edx, edx; mov r10, [rcx+0x*]")
        .Find("mov r10, [rcx+0x*]")
        .Or()
        .VirtualMatch(TEXT("/Script/Engine.Default__StaticMeshComponent"), "test r8b, r8b; jz 0x*; mov rax, [rcx+0x*]; test rax, rax; jnz 0x*")
        .Or()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call [*+0x*]", 2)
        .Or()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call 0x*; test rax, rax; jnz *")
        .Find("jnz 0x*")
        .Target()
        .Find("mov r9d, e*; mov r8b, 0x1; xor edx, edx; mov r10, [rcx+0x*]")
        .Find("mov r10, [rcx+0x*]")
        .Get();

    return GetBodyInstance(this, BoneName, bGetWelded, INDEX_NONE);
}

void UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::Type NewType)
{
    static auto SetCollisionEnabled = Anchor::VirtualFunction<void (*)(UPrimitiveComponent*, ECollisionEnabled::Type)>(TEXT("UPrimitiveComponent::SetCollisionEnabled"))
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetCollisionEnabled"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetCollisionEnabled"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.PrimitiveComponent:SetCollisionEnabled"))
        .Find("call [*+0x*]")
        .Get();

    SetCollisionEnabled(this, NewType);
}
