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
