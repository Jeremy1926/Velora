#pragma once

#include "GameFramework/GameMode.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "UObject/AssetPtr.h"

class ABuildingActor;
class AFortGameSession;
class AFortPlayerController;
class UFortGameDeathPenalty;
class UFortPlaylistManager;

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

    TAssetPtr<UFortGameDeathPenalty>& GetDeathPenaltyData();

    UFortPlaylistManager* GetPlaylistManager() const;

    FName GetCurrentPlaylistName() const;
};
