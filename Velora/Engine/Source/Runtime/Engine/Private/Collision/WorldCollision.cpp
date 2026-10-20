#include "pch.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Math/Quat.h"
#include "Scanner.h"


FCollisionQueryParams& FCollisionQueryParams::DefaultQueryParam = *(FCollisionQueryParams*)Scanner::String(TEXT("HeldObject.Placement.Mesh")).Start().Find("lea rax, [0x*]; mov [rsp+0x38], rax").Target();

FCollisionResponseParams& FCollisionResponseParams::DefaultResponseParam = *(FCollisionResponseParams*)Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceSingle")).Find("mov dword ptr [rsp+0x50], 0x2").Last("lea rcx, [0x*]").Target();

bool UWorld::SweepSingleByChannel(FHitResult& OutHit, const FVector& Start, const FVector& End, const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const FCollisionResponseParams& ResponseParam) const
{
    static auto SweepSingleByChannel = Anchor::Function<bool (*)(const UWorld*, FHitResult&, const FVector&, const FVector&, const FQuat&, ECollisionChannel, const FCollisionShape&, const FCollisionQueryParams&, const FCollisionResponseParams&)>(TEXT("UWorld::SweepSingleByChannel"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceSingle"))
        .Find("mov dword ptr [rsp+0x50], 0x2")
        .Find("call 0x*")
        .Target()
        .Get();

    return SweepSingleByChannel(this, OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);
}
