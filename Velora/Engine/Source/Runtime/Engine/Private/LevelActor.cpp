#include "pch.h"
#include "Engine/World.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FActorSpawnParameters::FActorSpawnParameters()
    : Name(NAME_None)
    , Template(nullptr)
    , Owner(nullptr)
    , Instigator(nullptr)
    , OverrideLevel(nullptr)
    , ObjectFlags(RF_Transactional)
{
    memset(&SpawnCollisionHandlingOverride, 0, (uint8*)&ObjectFlags - (uint8*)&SpawnCollisionHandlingOverride);
}

AActor* UWorld::SpawnActor(UClass* Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters)
{
    static auto SpawnActor = Anchor::Function<AActor* (*)(UWorld*, UClass*, FTransform const*, const FActorSpawnParameters&)>(TEXT("UWorld::SpawnActor"))
        .String(TEXT("SpawnActor failed because no class was specified"))
        .Start()
        .Get();

    return SpawnActor(this, Class, UserTransformPtr, SpawnParameters);
}

AActor* UWorld::SpawnActor(UClass* Class, FVector const* Location, FRotator const* Rotation, const FActorSpawnParameters& SpawnParameters)
{
    FTransform Transform;

    if (Location)
        Transform.SetLocation(*Location);

    if (Rotation)
        Transform.SetRotation(FQuat(*Rotation));

    return SpawnActor(Class, &Transform, SpawnParameters);
}
