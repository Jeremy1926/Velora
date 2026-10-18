#pragma once

#include "FortGamePvPBase.h"

class AFortGameModeAthena : public AFortGamePvPBase
{
public:
    static UClass* StaticClass();

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

    void OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions);

    FName RedirectLootTierGroup(FName LootTierGroup) const;

    void StartSafeZones();

    void TrySupplyDrop();
};
