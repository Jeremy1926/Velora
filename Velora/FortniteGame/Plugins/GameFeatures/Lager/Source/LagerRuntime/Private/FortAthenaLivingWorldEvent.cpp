#include "pch.h"
#include "FortAthenaLivingWorldEvent.h"
#include "FortAthenaLivingWorldEventData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortAthenaLivingWorldEventDespawnCondition::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEventDespawnCondition"));

    return Struct;
}

bool FFortAthenaLivingWorldEventDespawnCondition::GetbCanBeDespawned()
{
    static auto bCanBeDespawned = StaticStruct()->FindPropertyByName(FName(TEXT("bCanBeDespawned")));

    return *bCanBeDespawned->ContainerPtrToValuePtr<bool>(this);
}

float FFortAthenaLivingWorldEventDespawnCondition::GetMinDistanceToPlayer()
{
    static auto MinDistanceToPlayer = StaticStruct()->FindPropertyByName(FName(TEXT("MinDistanceToPlayer")));

    return *MinDistanceToPlayer->ContainerPtrToValuePtr<float>(this);
}

float FFortAthenaLivingWorldEventDespawnCondition::GetTimeAsCandidateBeforeDespawn()
{
    static auto TimeAsCandidateBeforeDespawn = StaticStruct()->FindPropertyByName(FName(TEXT("TimeAsCandidateBeforeDespawn")));

    return *TimeAsCandidateBeforeDespawn->ContainerPtrToValuePtr<float>(this);
}

EAthenaGamePhaseStep FFortAthenaLivingWorldEventDespawnCondition::GetCanStartDespawningPhaseStep()
{
    static auto CanStartDespawningPhaseStep = StaticStruct()->FindPropertyByName(FName(TEXT("CanStartDespawningPhaseStep")));

    return *CanStartDespawningPhaseStep->ContainerPtrToValuePtr<EAthenaGamePhaseStep>(this);
}

int32* FFortAthenaLivingWorldEventDespawnCondition::GetCanStartDespawningSafeZonePhase()
{
    static auto CanStartDespawningSafeZonePhase = StaticStruct()->FindPropertyByName(FName(TEXT("CanStartDespawningSafeZonePhase")));

    return CanStartDespawningSafeZonePhase ? CanStartDespawningSafeZonePhase->ContainerPtrToValuePtr<int32>(this) : nullptr;
}

bool FFortAthenaLivingWorldEventDespawnCondition::GetbCanDespawnWhenInCombatWithPlayer()
{
    static auto bCanDespawnWhenInCombatWithPlayer = StaticStruct()->FindPropertyByName(FName(TEXT("bCanDespawnWhenInCombatWithPlayer")));

    return *bCanDespawnWhenInCombatWithPlayer->ContainerPtrToValuePtr<bool>(this);
}

bool* FFortAthenaLivingWorldEventDespawnCondition::GetbCanDespawnInTheStorm()
{
    static auto bCanDespawnInTheStorm = StaticStruct()->FindPropertyByName(FName(TEXT("bCanDespawnInTheStorm")));

    return bCanDespawnInTheStorm ? bCanDespawnInTheStorm->ContainerPtrToValuePtr<bool>(this) : nullptr;
}

UScriptStruct* FFortAthenaLivingWorldEvent::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEvent"));

    return Struct;
}

bool FFortAthenaLivingWorldEvent::IsActive(const FGameplayTagContainer& PlaylistContextTags)
{
    if (!GetRequirePlaylistTags().IsEmpty() && !PlaylistContextTags.HasAny(GetRequirePlaylistTags()))
        return false;

    return GetIsEnabled().GetValueAtLevel(0.0f) > 0.0f;
}

int32 FFortAthenaLivingWorldEvent::GetMaxSpawnedCount(int32 SafeZonePhase)
{
    auto& MaxSpawnedCount = GetMaxSpawnedCount();

    if (!MaxSpawnedCount.Num())
        return 0;

    return (int32)MaxSpawnedCount[SafeZonePhase < MaxSpawnedCount.Num() ? SafeZonePhase : MaxSpawnedCount.Num() - 1];
}

TSoftObjectPtr<UFortAthenaLivingWorldEventData>& FFortAthenaLivingWorldEvent::GetEventData()
{
    static auto EventData = StaticStruct()->FindPropertyByName(FName(TEXT("EventData")));

    return *EventData->ContainerPtrToValuePtr<TSoftObjectPtr<UFortAthenaLivingWorldEventData>>(this);
}

FScalableFloat& FFortAthenaLivingWorldEvent::GetIsEnabled()
{
    static auto IsEnabled = StaticStruct()->FindPropertyByName(FName(TEXT("IsEnabled")));

    return *IsEnabled->ContainerPtrToValuePtr<FScalableFloat>(this);
}

TArray<float>& FFortAthenaLivingWorldEvent::GetMaxCount()
{
    static auto MaxCount = StaticStruct()->FindPropertyByName(FName(TEXT("MaxCount")));

    return *MaxCount->ContainerPtrToValuePtr<TArray<float>>(this);
}

TArray<float>& FFortAthenaLivingWorldEvent::GetMaxSpawnedCount()
{
    static auto MaxSpawnedCount = StaticStruct()->FindPropertyByName(FName(TEXT("MaxSpawnedCount")));

    return *MaxSpawnedCount->ContainerPtrToValuePtr<TArray<float>>(this);
}

float& FFortAthenaLivingWorldEvent::GetWeight()
{
    static auto Weight = StaticStruct()->FindPropertyByName(FName(TEXT("Weight")));

    return *Weight->ContainerPtrToValuePtr<float>(this);
}

bool& FFortAthenaLivingWorldEvent::GetbOnlyInSafeZone()
{
    static auto bOnlyInSafeZone = StaticStruct()->FindPropertyByName(FName(TEXT("bOnlyInSafeZone")));

    return *bOnlyInSafeZone->ContainerPtrToValuePtr<bool>(this);
}

EAthenaGamePhaseStep& FFortAthenaLivingWorldEvent::GetActivationGamePhaseStep()
{
    static auto ActivationGamePhaseStep = StaticStruct()->FindPropertyByName(FName(TEXT("ActivationGamePhaseStep")));

    return *ActivationGamePhaseStep->ContainerPtrToValuePtr<EAthenaGamePhaseStep>(this);
}

int32& FFortAthenaLivingWorldEvent::GetActivationSafeZoneIndex()
{
    static auto ActivationSafeZoneIndex = StaticStruct()->FindPropertyByName(FName(TEXT("ActivationSafeZoneIndex")));

    return *ActivationSafeZoneIndex->ContainerPtrToValuePtr<int32>(this);
}

float& FFortAthenaLivingWorldEvent::GetActivationMinDelay()
{
    static auto ActivationMinDelay = StaticStruct()->FindPropertyByName(FName(TEXT("ActivationMinDelay")));

    return *ActivationMinDelay->ContainerPtrToValuePtr<float>(this);
}

float& FFortAthenaLivingWorldEvent::GetActivationMaxDelay()
{
    static auto ActivationMaxDelay = StaticStruct()->FindPropertyByName(FName(TEXT("ActivationMaxDelay")));

    return *ActivationMaxDelay->ContainerPtrToValuePtr<float>(this);
}

FGameplayTagContainer& FFortAthenaLivingWorldEvent::GetRequirePlaylistTags()
{
    static auto RequirePlaylistTags = StaticStruct()->FindPropertyByName(FName(TEXT("RequirePlaylistTags")));

    return *RequirePlaylistTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

bool FFortAthenaLivingWorldEvent::GetbCheckForActorDensity()
{
    static auto bCheckForActorDensity = StaticStruct()->FindPropertyByName(FName(TEXT("bCheckForActorDensity")));

    return bCheckForActorDensity ? *bCheckForActorDensity->ContainerPtrToValuePtr<bool>(this) : false;
}

bool FFortAthenaLivingWorldEvent::GetbOnlyInNextSafeZone()
{
    static auto bOnlyInNextSafeZone = StaticStruct()->FindPropertyByName(FName(TEXT("bOnlyInNextSafeZone")));

    return bOnlyInNextSafeZone ? *bOnlyInNextSafeZone->ContainerPtrToValuePtr<bool>(this) : false;
}

bool FFortAthenaLivingWorldEvent::GetbOnlyInNextSafeZoneIfStormShrinking()
{
    static auto bOnlyInNextSafeZoneIfStormShrinking = StaticStruct()->FindPropertyByName(FName(TEXT("bOnlyInNextSafeZoneIfStormShrinking")));

    return bOnlyInNextSafeZoneIfStormShrinking ? *bOnlyInNextSafeZoneIfStormShrinking->ContainerPtrToValuePtr<bool>(this) : false;
}

TArray<FFortAthenaLivingWorldEventTagDensityCondition>* FFortAthenaLivingWorldEvent::GetTagDensityConditions()
{
    static auto TagDensityConditions = StaticStruct()->FindPropertyByName(FName(TEXT("TagDensityConditions")));

    return TagDensityConditions ? TagDensityConditions->ContainerPtrToValuePtr<TArray<FFortAthenaLivingWorldEventTagDensityCondition>>(this) : nullptr;
}

float FFortAthenaLivingWorldEvent::GetMaxActorDensityOverride()
{
    static auto MaxActorDensityOverride = StaticStruct()->FindPropertyByName(FName(TEXT("MaxActorDensityOverride")));

    return MaxActorDensityOverride ? *MaxActorDensityOverride->ContainerPtrToValuePtr<float>(this) : -1.0f;
}

float FFortAthenaLivingWorldEvent::GetMinDistanceToPlayer()
{
    static auto MinDistanceToPlayer = StaticStruct()->FindPropertyByName(FName(TEXT("MinDistanceToPlayer")));

    return MinDistanceToPlayer ? *MinDistanceToPlayer->ContainerPtrToValuePtr<float>(this) : 0.0f;
}

float FFortAthenaLivingWorldEvent::GetMaxDistanceToPlayer()
{
    static auto MaxDistanceToPlayer = StaticStruct()->FindPropertyByName(FName(TEXT("MaxDistanceToPlayer")));

    return MaxDistanceToPlayer ? *MaxDistanceToPlayer->ContainerPtrToValuePtr<float>(this) : 0.0f;
}

bool FFortAthenaLivingWorldEvent::GetbCanSpawnWithoutMatchingPlayerDistance()
{
    static auto bCanSpawnWithoutMatchingPlayerDistance = StaticStruct()->FindPropertyByName(FName(TEXT("bCanSpawnWithoutMatchingPlayerDistance")));

    return bCanSpawnWithoutMatchingPlayerDistance ? *bCanSpawnWithoutMatchingPlayerDistance->ContainerPtrToValuePtr<bool>(this) : false;
}

FFortAthenaLivingWorldEventDespawnCondition* FFortAthenaLivingWorldEvent::GetDespawnCondition()
{
    static auto DespawnCondition = StaticStruct()->FindPropertyByName(FName(TEXT("DespawnCondition")));

    return DespawnCondition ? DespawnCondition->ContainerPtrToValuePtr<FFortAthenaLivingWorldEventDespawnCondition>(this) : nullptr;
}
