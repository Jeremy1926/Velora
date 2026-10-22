#include "pch.h"
#include "FortAthenaLivingWorldEventData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortAthenaLivingWorldEventDataActorSpawnDescription::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEventDataActorSpawnDescription"));

    return Struct;
}

int32 FFortAthenaLivingWorldEventDataActorSpawnDescription::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

TSoftClassPtr<UClass>& FFortAthenaLivingWorldEventDataActorSpawnDescription::GetActorClass()
{
    static auto ActorClass = StaticStruct()->FindPropertyByName(FName(TEXT("ActorClass")));

    return *ActorClass->ContainerPtrToValuePtr<TSoftClassPtr<UClass>>(this);
}

TSoftClassPtr<UClass>& FFortAthenaLivingWorldEventDataActorSpawnDescription::GetSpawnerData()
{
    static auto SpawnerData = StaticStruct()->FindPropertyByName(FName(TEXT("SpawnerData")));

    return *SpawnerData->ContainerPtrToValuePtr<TSoftClassPtr<UClass>>(this);
}

FGameplayTagQuery& FFortAthenaLivingWorldEventDataActorSpawnDescription::GetSpawnerDataTagQuery()
{
    static auto SpawnerDataTagQuery = StaticStruct()->FindPropertyByName(FName(TEXT("SpawnerDataTagQuery")));

    return *SpawnerDataTagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}

bool FFortAthenaLivingWorldEventDataActorSpawnDescription::GetbSpawnAroundDefaultPoint()
{
    static auto bSpawnAroundDefaultPoint = StaticStruct()->FindPropertyByName(FName(TEXT("bSpawnAroundDefaultPoint")));

    return bSpawnAroundDefaultPoint && *bSpawnAroundDefaultPoint->ContainerPtrToValuePtr<bool>(this);
}

UEnvQuery* FFortAthenaLivingWorldEventDataActorSpawnDescription::GetSpawnAroundEnvironmentQuery()
{
    static auto SpawnAroundEnvironmentQuery = StaticStruct()->FindPropertyByName(FName(TEXT("SpawnAroundEnvironmentQuery")));

    return SpawnAroundEnvironmentQuery ? *SpawnAroundEnvironmentQuery->ContainerPtrToValuePtr<UEnvQuery*>(this) : nullptr;
}

UClass* UFortAthenaLivingWorldEventData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEventData"));

    return Class;
}

FGameplayTagQuery& UFortAthenaLivingWorldEventData::GetProviderFiltersTagQuery()
{
    static auto ProviderFiltersTagQuery = GetClass()->FindPropertyByName(FName(TEXT("ProviderFiltersTagQuery")));

    return *ProviderFiltersTagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}

FScalableFloat* UFortAthenaLivingWorldEventData::GetMinActorCountToSpawn()
{
    static auto MinActorCountToSpawn = GetClass()->FindPropertyByName(FName(TEXT("MinActorCountToSpawn")));

    return MinActorCountToSpawn ? MinActorCountToSpawn->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

FScalableFloat* UFortAthenaLivingWorldEventData::GetMaxActorCountToSpawn()
{
    static auto MaxActorCountToSpawn = GetClass()->FindPropertyByName(FName(TEXT("MaxActorCountToSpawn")));

    return MaxActorCountToSpawn ? MaxActorCountToSpawn->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

TRuntimeSizedArray<FFortAthenaLivingWorldEventDataActorSpawnDescription>& UFortAthenaLivingWorldEventData::GetActorDescriptions()
{
    static auto ActorDescriptions = GetClass()->FindPropertyByName(FName(TEXT("ActorDescriptions")));

    return *ActorDescriptions->ContainerPtrToValuePtr<TRuntimeSizedArray<FFortAthenaLivingWorldEventDataActorSpawnDescription>>(this);
}
