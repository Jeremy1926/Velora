#include "pch.h"
#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const FTransform& USceneComponent::GetComponentTransform() const
{
    static auto ComponentToWorld = Scanner::Exec(TEXT("/Script/Engine.Actor:K2_GetActorLocation")).Find("mov?ps xmm*, [*+0x*]").Disp(TEXT("USceneComponent::ComponentToWorld.Translation")) - (int32_t)sizeof(VectorRegister);

    return *(FTransform*)((uint8_t*)this + ComponentToWorld);
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
    static auto Bounds = Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds")).Find("movsd xmm0, qword ptr [*]").Disp(TEXT("USceneComponent::Bounds"));

    return *(FBoxSphereBounds*)((uint8_t*)this + Bounds);
}

FBoxSphereBounds USceneComponent::CalcLocalBounds() const
{
    static auto CalcLocalBounds = Anchor::Slot(TEXT("USceneComponent::CalcLocalBounds"))
        .String(TEXT("HeldObject.Placement.Mesh"))
        .Start()
        .Find("mov rax, [rcx]; call [rax+0x*]; lea rcx, [*+0x*]; call 0x*; mov rcx, [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    FBoxSphereBounds Result;

    return *Scanner::Virtual<FBoxSphereBounds* (*)(const USceneComponent*, FBoxSphereBounds*)>(this, CalcLocalBounds)(this, &Result);
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
    static auto GetSocketLocation = Anchor::Slot(TEXT("USceneComponent::GetSocketLocation"))
        .Exec(TEXT("/Script/Engine.SceneComponent:GetSocketLocation"))
        .Find("call [*+0x*]")
        .Get();

    FVector Result;

    return *Scanner::Virtual<FVector* (*)(const USceneComponent*, FVector*, FName)>(this, GetSocketLocation)(this, &Result, InSocketName);
}
