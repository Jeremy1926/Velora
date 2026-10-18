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
    , SpawnCollisionHandlingOverride(ESpawnActorCollisionHandlingMethod::Undefined)
    , bRemoteOwned(false)
    , bNoFail(false)
    , bDeferConstruction(false)
    , bAllowDuringConstructionScript(false)
    , ObjectFlags(RF_Transactional)
{
}

AActor* UWorld::SpawnActor(UClass* Class, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters)
{
    static auto SpawnActor = Scanner::Function<AActor* (*)(UWorld*, UClass*, FTransform const*, const FActorSpawnParameters&)>(Scanner::String(TEXT("SpawnActor failed because no class was specified")).Start().Get(TEXT("UWorld::SpawnActor")));

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
