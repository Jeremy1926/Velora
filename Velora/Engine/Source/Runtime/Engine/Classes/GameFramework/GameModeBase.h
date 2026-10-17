#pragma once

#include "GameFramework/Info.h"

class AController;
class APawn;

class AGameModeBase : public AInfo
{
public:
    UClass* GetDefaultPawnClassForController(AController* InController);

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);
};
