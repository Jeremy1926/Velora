#include "pch.h"
#include "Engine/World.h"
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
    static auto SpawnActor = (AActor* (*)(UWorld*, UClass*, FTransform const*, const FActorSpawnParameters&))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("SpawnActor failed because no class was specified")))->BeginAddress);

    return SpawnActor(this, Class, UserTransformPtr, SpawnParameters);
}
