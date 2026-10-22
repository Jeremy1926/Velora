#pragma once

#include "Engine/DataTable.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

class UFortAthenaLivingWorldEventData;
enum class EAthenaGamePhaseStep : uint8;

struct FFortAthenaLivingWorldEvent : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    bool IsActive(const FGameplayTagContainer& PlaylistContextTags);

    int32 GetMaxSpawnedCount(int32 SafeZonePhase);

    TSoftObjectPtr<UFortAthenaLivingWorldEventData>& GetEventData();

    FScalableFloat& GetIsEnabled();

    TArray<float>& GetMaxCount();

    TArray<float>& GetMaxSpawnedCount();

    float& GetWeight();

    bool& GetbOnlyInSafeZone();

    EAthenaGamePhaseStep& GetActivationGamePhaseStep();

    int32& GetActivationSafeZoneIndex();

    float& GetActivationMinDelay();

    float& GetActivationMaxDelay();

    FGameplayTagContainer& GetRequirePlaylistTags();

    bool GetbCheckForActorDensity();
};
