#include "pch.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "WorldCollision.h"
#include "Math/Quat.h"
#include "Scanner.h"

FCollisionQueryParams& FCollisionQueryParams::DefaultQueryParam = *Anchor::Data<FCollisionQueryParams*>(TEXT("FCollisionQueryParams::DefaultQueryParam"))
    .String("DefaultQueryParam")
    .Start()
    .Find("lea rcx, [0x*]; call 0x*")
    .Target()
    .Get();

FCollisionResponseParams& FCollisionResponseParams::DefaultResponseParam = *Anchor::Data<FCollisionResponseParams*>(TEXT("FCollisionResponseParams::DefaultResponseParam"))
    .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceSingle"))
    .Find("mov dword ptr [rsp+0x5?], 0x2")
    .Last("lea rcx, [0x*]")
    .Target()
    .Get();

bool UWorld::SweepSingleByChannel(FHitResult& OutHit, const FVector& Start, const FVector& End, const FQuat& Rot, ECollisionChannel TraceChannel, const FCollisionShape& CollisionShape, const FCollisionQueryParams& Params, const FCollisionResponseParams& ResponseParam) const
{
    static auto SweepSingleByChannel = Anchor::Function<bool (*)(const UWorld*, FHitResult&, const FVector&, const FVector&, const FQuat&, ECollisionChannel, const FCollisionShape&, const FCollisionQueryParams&, const FCollisionResponseParams&)>(TEXT("UWorld::SweepSingleByChannel"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:SphereTraceSingle"))
        .Find("mov dword ptr [rsp+0x5?], 0x2")
        .Find("call 0x*")
        .Target()
        .Get();

    return SweepSingleByChannel(this, OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);
}
