#include "pch.h"
#include "Items/FortVehicleItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortVehicleItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleItemDefinition"));

    return Class;
}

const FScalableFloat& UFortVehicleItemDefinition::GetVehicleMinSpawnPercent() const
{
    static auto VehicleMinSpawnPercent = StaticClass()->FindPropertyByName(FName(TEXT("VehicleMinSpawnPercent")));

    return *VehicleMinSpawnPercent->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& UFortVehicleItemDefinition::GetVehicleMaxSpawnPercent() const
{
    static auto VehicleMaxSpawnPercent = StaticClass()->FindPropertyByName(FName(TEXT("VehicleMaxSpawnPercent")));

    return *VehicleMaxSpawnPercent->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& UFortVehicleItemDefinition::GetMinPercentWithGas() const
{
    static auto MinPercentWithGas = StaticClass()->FindPropertyByName(FName(TEXT("MinPercentWithGas")));

    return *MinPercentWithGas->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& UFortVehicleItemDefinition::GetMaxPercentWithGas() const
{
    static auto MaxPercentWithGas = StaticClass()->FindPropertyByName(FName(TEXT("MaxPercentWithGas")));

    return *MaxPercentWithGas->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& UFortVehicleItemDefinition::GetMinPercentInoperable() const
{
    static auto MinPercentInoperable = StaticClass()->FindPropertyByName(FName(TEXT("MinPercentInoperable")));

    return *MinPercentInoperable->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& UFortVehicleItemDefinition::GetMaxPercentInoperable() const
{
    static auto MaxPercentInoperable = StaticClass()->FindPropertyByName(FName(TEXT("MaxPercentInoperable")));

    return *MaxPercentInoperable->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const TSoftClassPtr<UClass>& UFortVehicleItemDefinition::GetVehicleActorClass() const
{
    static auto VehicleActorClass = StaticClass()->FindPropertyByName(FName(TEXT("VehicleActorClass")));

    return *VehicleActorClass->ContainerPtrToValuePtr<TSoftClassPtr<UClass>>(this);
}
