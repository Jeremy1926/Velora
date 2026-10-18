#include "pch.h"
#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const FTransform& USceneComponent::GetComponentTransform() const
{
    static auto ComponentToWorld = Scanner::Exec(TEXT("/Script/Engine.Actor:K2_GetActorLocation")).Find("movaps xmm*, [*+0x*]").Disp() - (int32_t)sizeof(VectorRegister);

    return *(FTransform*)((uint8_t*)this + ComponentToWorld);
}

const FBoxSphereBounds& USceneComponent::GetBounds() const
{
    static auto Bounds = Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:GetComponentBounds")).Find("movsd xmm0, qword ptr [*]").Disp();

    return *(FBoxSphereBounds*)((uint8_t*)this + Bounds);
}
