#pragma once

#include "GameFramework/GameMode.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"

class ABuildingActor;
class AFortGameSession;
class AFortPlayerController;

class AFortGameMode : public AGameMode
{
public:
    static UClass* StaticClass();

    void OnPlayerCreatedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor);

    void OnPlayerRepairedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor);

    void OnPlayerEditedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor);

    void FinishWorldInitialization();

    void OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions);

    AFortGameSession* GetFortGameSession();
};
