#pragma once

#include "GameFramework/Actor.h"

class AAbandonedCampsitePlacedSpawner : public AActor
{
public:
    static UClass* StaticClass();

    void SpawnCampsite();
};
