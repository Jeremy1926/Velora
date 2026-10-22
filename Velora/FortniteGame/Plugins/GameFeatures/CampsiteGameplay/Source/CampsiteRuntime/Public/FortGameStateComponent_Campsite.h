#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"

class UObject;
class AAbandonedCampsitePlacedSpawner;

class UFortGameStateComponent_Campsite : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_Campsite* Get(const UObject* WorldContextObject);

    TArray<AAbandonedCampsitePlacedSpawner*>& GetPlacedCampsiteSpawnPoints();
};
