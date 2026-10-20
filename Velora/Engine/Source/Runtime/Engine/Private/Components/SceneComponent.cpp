#include "pch.h"
#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const FTransform& USceneComponent::GetComponentTransform() const
{
    static auto ComponentToWorld = Anchor::Offset(TEXT("USceneComponent::ComponentToWorld.Translation"))
        .Exec(TEXT("/Script/Engine.Actor:K2_GetActorLocation"))
        .Find("mov?ps xmm*, [*+0x*]")
        .Disp();

    return *(FTransform*)((uint8_t*)this + ComponentToWorld - (int32_t)sizeof(VectorRegister));
}

FRotator USceneComponent::GetComponentRotation() const
{
    struct FRotationConversionCache
    {
        FQuat CachedQuat;
        FRotator CachedRotator;
    };

    auto& WorldRotationCache = *(FRotationConversionCache*)((uint8_t*)&GetComponentTransform() + sizeof(FTransform));
    auto InNormalizedQuat = GetComponentTransform().GetRotation();

    if (InNormalizedQuat != WorldRotationCache.CachedQuat)
    {
        WorldRotationCache.CachedQuat = InNormalizedQuat;
        WorldRotationCache.CachedRotator = InNormalizedQuat.Rotator();
    }

    return WorldRotationCache.CachedRotator;
}

const FBoxSphereBounds& USceneComponent::GetBounds() const
{
    static auto Bounds = Anchor::Offset(TEXT("USceneComponent::Bounds"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds"))
        .Find("movsd xmm0, qword ptr [*]")
        .Disp();

    return *(FBoxSphereBounds*)((uint8_t*)this + Bounds);
}

FBoxSphereBounds USceneComponent::CalcLocalBounds() const
{
    static auto CalcLocalBounds = Anchor::VirtualFunction<FBoxSphereBounds* (*)(const USceneComponent*, FBoxSphereBounds*)>(TEXT("USceneComponent::CalcLocalBounds"))
        .String(TEXT("HeldObject.Placement.Mesh"))
        .Start()
        .Find("mov rax, [rcx]; call [rax+0x*]; lea rcx, [*+0x*]; call 0x*; mov rcx, [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    FBoxSphereBounds Result;

    return *CalcLocalBounds(this, &Result);
}

void USceneComponent::K2_SetWorldLocationAndRotation(FVector NewLocation, FRotator NewRotation, bool bSweep, bool bTeleport)
{
    static auto K2_SetWorldLocationAndRotation = Anchor::Function<void (*)(USceneComponent*, FVector, FRotator, bool, void*, bool)>(TEXT("USceneComponent::K2_SetWorldLocationAndRotation"))
        .Native(TEXT("/Script/Engine.SceneComponent:K2_SetWorldLocationAndRotation"))
        .Get();

    K2_SetWorldLocationAndRotation(this, NewLocation, NewRotation, bSweep, nullptr, bTeleport);
}

FVector USceneComponent::GetSocketLocation(FName InSocketName) const
{
    static auto GetSocketLocation = Anchor::VirtualFunction<FVector* (*)(const USceneComponent*, FVector*, FName)>(TEXT("USceneComponent::GetSocketLocation"))
        .Exec(TEXT("/Script/Engine.SceneComponent:GetSocketLocation"))
        .Find("call [*+0x*]")
        .Get();

    FVector Result;

    return *GetSocketLocation(this, &Result, InSocketName);
}
