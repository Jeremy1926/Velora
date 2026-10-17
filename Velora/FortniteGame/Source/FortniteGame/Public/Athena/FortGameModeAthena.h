#pragma once

#include "FortGamePvPBase.h"

class AFortGameModeAthena : public AFortGamePvPBase
{
public:
    static UClass* StaticClass();

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);
};
