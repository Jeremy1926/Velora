#pragma once

#include "Components/ActorComponent.h"
#include "Items/FortVictoryCrownAccountItem.h"
#include "UObject/ScriptDelegates.h"

struct FFrame;
class APawn;
class AFortPlayerController;
class UFortWorldItem;

class UFortControllerComponent_VictoryCrowns : public UActorComponent
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void execHandlePawnChanged(FFrame& Stack, void* const Z_Param__Result);

    void execOnAthenaProfileInitialized(FFrame& Stack, void* const Z_Param__Result);

    void execCacheAccountItemData(FFrame& Stack, void* const Z_Param__Result);

    void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

    void HandlePlayerPlaceChanged();

    bool WasOwnerQualifyingWinner() const;

    void SetWonCrownInMatch(bool bNewWonCrownInMatch);

    void SetWonRoyalRoyale(bool bNewWonRoyalRoyale);

    void BindOnAthenaProfileInitialized();

    void OnAthenaProfileInitialized();

    void CacheAccountItemData();

    void SetCrownAccountData(const FVictoryCrownAccountData& NewCrownAccountData);

    bool CurrentPlaylistSupportsVictoryCrowns() const;

    uint8* FindCurrentPlaylistRow() const;

    void GrantVictoryCrownItem();

    const UFortWorldItem* GetCrownInPlayerInventory() const;

    AFortPlayerController* GetPlayerController() const;

    bool& GetShouldGrantCrownOnSpawn();

    FVictoryCrownAccountData& GetVictoryCrownAccountData();

    bool& GetWonCrownInMatch();

    bool& GetWonRoyalRoyale();

    FMulticastScriptDelegate& GetOnVictoryCrownStatusChanged();
};
