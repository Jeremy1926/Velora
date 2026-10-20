#include "pch.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Math/Quat.h"
#include "Scanner.h"

static Scanner::FCode SphereTraceSingleSweep()
{
    static auto SphereTraceSingleSweep = Scanner::Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceSingle")).Find("mov dword ptr [rsp+0x50], 0x2");

    return SphereTraceSingleSweep;
}

FCollisionQueryParams& FCollisionQueryParams::DefaultQueryParam = *(FCollisionQueryParams*)Scanner::String(TEXT("HeldObject.Placement.Mesh")).Start().Find("lea rax, [0x*]; mov [rsp+0x38], rax").Target();

FCollisionResponseParams& FCollisionResponseParams::DefaultResponseParam = *(FCollisionResponseParams*)SphereTraceSingleSweep().Last("lea rcx, [0x*]").Target();

bool UWorld::SweepSingleByChannel(FHitResult& OutHit, const FVector& Start, const FVector& End, const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const FCollisionResponseParams& ResponseParam) const
{
    static auto SweepSingleByChannel = Scanner::Function<bool (*)(const UWorld*, FHitResult&, const FVector&, const FVector&, const FQuat&, ECollisionChannel, const FCollisionShape&, const FCollisionQueryParams&, const FCollisionResponseParams&)>(SphereTraceSingleSweep().Find("call 0x*").Target(TEXT("UWorld::SweepSingleByChannel")));

    return SweepSingleByChannel(this, OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);
}
