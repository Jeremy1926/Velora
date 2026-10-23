#include "pch.h"
#include "Engine/World.h"
#include "Math/TransformVectorized.h"
#include "Misc/EngineVersion.h"
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

    if (FEngineVersion::Current().GetMajor() < 5)
        return SpawnActor(this, Class, UserTransformPtr, SpawnParameters);

    alignas(FActorSpawnParameters) uint8 Parameters[0x90] = {};

    memcpy(Parameters, &SpawnParameters, 0x28);
    *(ESpawnActorCollisionHandlingMethod*)(Parameters + 0x30) = SpawnParameters.SpawnCollisionHandlingOverride;
    *(EObjectFlags*)(Parameters + 0x34) = SpawnParameters.ObjectFlags;

    return SpawnActor(this, Class, UserTransformPtr, *(const FActorSpawnParameters*)Parameters);
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
