#pragma once

#include "GameFramework/GameMode.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "UObject/AssetPtr.h"

class ABuildingActor;
class UDataTable;
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

    bool GetWorldIsReady() const;

    AFortGameSession* GetFortGameSession();

    TAssetPtr<UFortGameDeathPenalty>& GetDeathPenaltyData();

    UFortPlaylistManager* GetPlaylistManager() const;

    FName GetCurrentPlaylistName() const;

    void GetLootTierDataTables(TArray<UDataTable*>& OutLootTierDataTables) const;

    void GetLootPackageDataTables(TArray<UDataTable*>& OutLootPackageDataTables) const;
};
