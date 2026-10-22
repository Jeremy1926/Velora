#include "pch.h"
#include "FortAthenaLivingWorldConfigData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAthenaLivingWorldConfigData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldConfigData"));

    return Class;
}

TSoftObjectPtr<UDataTable>& UFortAthenaLivingWorldConfigData::GetCategoryTable()
{
    static auto CategoryTable = GetClass()->FindPropertyByName(FName(TEXT("CategoryTable")));

    return *CategoryTable->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

FScalableFloat& UFortAthenaLivingWorldConfigData::GetMaxActorCount()
{
    static auto MaxActorCount = GetClass()->FindPropertyByName(FName(TEXT("MaxActorCount")));

    return *MaxActorCount->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& UFortAthenaLivingWorldConfigData::GetMaxEventSpawnPerTick()
{
    static auto MaxEventSpawnPerTick = GetClass()->FindPropertyByName(FName(TEXT("MaxEventSpawnPerTick")));

    return *MaxEventSpawnPerTick->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& UFortAthenaLivingWorldConfigData::GetMinimumEventGenerationInterval()
{
    static auto MinimumEventGenerationInterval = GetClass()->FindPropertyByName(FName(TEXT("MinimumEventGenerationInterval")));

    return *MinimumEventGenerationInterval->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat* UFortAthenaLivingWorldConfigData::GetDensityComputationDistance()
{
    static auto DensityComputationDistance = GetClass()->FindPropertyByName(FName(TEXT("DensityComputationDistance")));

    return DensityComputationDistance ? DensityComputationDistance->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

FScalableFloat* UFortAthenaLivingWorldConfigData::GetMaxActorDensity()
{
    static auto MaxActorDensity = GetClass()->FindPropertyByName(FName(TEXT("MaxActorDensity")));

    return MaxActorDensity ? MaxActorDensity->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}
