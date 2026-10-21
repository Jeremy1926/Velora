#include "pch.h"
#include "FortValetGameFeatureData.h"
#include "Engine/EngineTypes.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortValetGameFeatureData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ValetRuntime.FortValetGameFeatureData"));

    return Class;
}

const TMap<TSoftClassPtr<ABuildingProp>, FVehiclePropReplacementData>& UFortValetGameFeatureData::GetPropReplacements() const
{
    static auto PropReplacements = StaticClass()->FindPropertyByName(FName(TEXT("PropReplacements")));

    return *PropReplacements->ContainerPtrToValuePtr<TMap<TSoftClassPtr<ABuildingProp>, FVehiclePropReplacementData>>(this);
}

const TSoftClassPtr<UClass>& UFortValetGameFeatureData::GetVehicleSpawnerClass() const
{
    static auto VehicleSpawnerClass = StaticClass()->FindPropertyByName(FName(TEXT("VehicleSpawnerClass")));

    return *VehicleSpawnerClass->ContainerPtrToValuePtr<TSoftClassPtr<UClass>>(this);
}

ESpawnActorCollisionHandlingMethod UFortValetGameFeatureData::GetSpawnActorCollisionHandlingMethod() const
{
    static auto SpawnActorCollisionHandlingMethod = StaticClass()->FindPropertyByName(FName(TEXT("SpawnActorCollisionHandlingMethod")));

    return *SpawnActorCollisionHandlingMethod->ContainerPtrToValuePtr<ESpawnActorCollisionHandlingMethod>(this);
}
