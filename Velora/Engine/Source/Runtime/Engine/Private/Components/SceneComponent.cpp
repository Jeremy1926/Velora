#include "pch.h"
#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FTransform USceneComponent::GetComponentTransform() const
{
    static auto ComponentToWorld = Anchor::Offset(TEXT("USceneComponent::ComponentToWorld.Translation"))
        .Exec(TEXT("/Script/Engine.Actor:K2_GetActorLocation"))
        .Find("mov?ps xmm*, [*+0x*]")
        .Disp();

    return TGameLayout<FTransform>::Load((const uint8*)this + ComponentToWorld - (UE::LWC::IsEnabled() ? (int32)sizeof(FQuat4d) : (int32)sizeof(VectorRegister)));
}

FRotator USceneComponent::GetComponentRotation() const
{
    if (UE::LWC::IsEnabled())
        return GetComponentTransform().GetRotation().Rotator();

    static auto ComponentToWorld = Anchor::Offset(TEXT("USceneComponent::ComponentToWorld.Translation"))
        .Exec(TEXT("/Script/Engine.Actor:K2_GetActorLocation"))
        .Find("mov?ps xmm*, [*+0x*]")
        .Disp();

    struct FRotationConversionCache
    {
        FQuat CachedQuat;
        FRotator CachedRotator;
    };

    auto& WorldRotationCache = *(FRotationConversionCache*)((uint8_t*)this + ComponentToWorld - (int32_t)sizeof(VectorRegister) + sizeof(FTransform));
    auto InNormalizedQuat = GetComponentTransform().GetRotation();

    if (InNormalizedQuat != WorldRotationCache.CachedQuat)
    {
        WorldRotationCache.CachedQuat = InNormalizedQuat;
        WorldRotationCache.CachedRotator = InNormalizedQuat.Rotator();
    }

    return WorldRotationCache.CachedRotator;
}

FBoxSphereBounds USceneComponent::GetBounds() const
{
    static auto Bounds = Anchor::Offset(TEXT("USceneComponent::Bounds"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds"))
        .Find("movsd xmm0, qword ptr [*]")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds"))
        .Find("movsd xmm0, qword ptr [rcx+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds"))
        .Find("movups xmm0, [rax+0x*]")
        .Disp();

    return TGameLayout<FBoxSphereBounds>::Load((const uint8*)this + Bounds);
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
    static auto K2_SetWorldLocationAndRotation = Anchor::Function<void (*)(USceneComponent*, TGameValue<FVector>, TGameValue<FRotator>, bool, void*, bool)>(TEXT("USceneComponent::K2_SetWorldLocationAndRotation"))
        .Native(TEXT("/Script/Engine.SceneComponent:K2_SetWorldLocationAndRotation"))
        .Get();

    K2_SetWorldLocationAndRotation(this, NewLocation, NewRotation, bSweep, nullptr, bTeleport);
}

FVector USceneComponent::GetSocketLocation(FName InSocketName) const
{
    static auto GetSocketLocation = Anchor::VirtualFunction<TGameValue<FVector>* (*)(const USceneComponent*, TGameValue<FVector>*, FName)>(TEXT("USceneComponent::GetSocketLocation"))
        .Exec(TEXT("/Script/Engine.SceneComponent:GetSocketLocation"))
        .Find("call [*+0x*]")
        .Get();

    TGameValue<FVector> Result;

    return GetSocketLocation(this, &Result, InSocketName)->Get();
}
