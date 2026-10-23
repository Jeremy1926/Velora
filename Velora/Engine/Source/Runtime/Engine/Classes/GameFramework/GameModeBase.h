#pragma once

#include "GameFramework/Info.h"
#include "Containers/UnrealString.h"

class AController;
class AGameStateBase;
class APawn;

class AGameModeBase : public AInfo
{
public:
    UClass* GetDefaultPawnClassForController(AController* InController);

    APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot);

    FString& GetOptionsString();

    AGameStateBase* GetGameState() const;
};
