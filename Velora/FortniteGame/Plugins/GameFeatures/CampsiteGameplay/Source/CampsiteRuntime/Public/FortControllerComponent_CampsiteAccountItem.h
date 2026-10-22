#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Items/FortInventory.h"

struct FFrame;
class AActor;
class AFortPlayerController;
class UFortWorldItem;

class UFortControllerComponent_CampsiteAccountItem : public UActorComponent
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void execOnAthenaProfileInitialized(FFrame& Stack, void* const Z_Param__Result);

    void execCacheAccountItemData(FFrame& Stack, void* const Z_Param__Result);

    void execStashCurrentlyHeldItemAndRemoveFromInventory(FFrame& Stack, void* const Z_Param__Result);

    void execHasCurrentlyStashedItem(FFrame& Stack, void* const Z_Param__Result);

    void execClearStashedItemAndGiveToPlayer(FFrame& Stack, void* const Z_Param__Result);

    void execSwapStashedItem(FFrame& Stack, void* const Z_Param__Result);

    void execClearStashedItem(FFrame& Stack, void* const Z_Param__Result);

    void execWriteStashedItem(FFrame& Stack, void* const Z_Param__Result);

    void execWriteStashedItemFromEntry(FFrame& Stack, void* const Z_Param__Result);

    void BindOnAthenaProfileInitialized();

    void OnAthenaProfileInitialized();

    void CacheAccountItemData();

    bool StashCurrentlyHeldItemAndRemoveFromInventory(int32 StashedItemIndex);

    bool HasCurrentlyStashedItem(int32 StashedItemIndex);

    bool ClearStashedItemAndGiveToPlayer(const AActor* SourceActor, int32 StashedItemIndex);

    bool SwapStashedItem(const AActor* SourceActor, int32 StashedItemIndex);

    void ClearStashedItem(int32 StashedItemIndex);

    void WriteStashedItem(UFortWorldItem* Item, int32 StashedItemIndex);

    void WriteStashedItemFromEntry(const FFortItemEntry& ItemEntry, int32 StashedItemIndex);

    UFortWorldItem* GetCurrentlyHeldItem();

    void GiveStashedItemToPlayer(const AActor* SourceActor, const FFortItemEntry& ItemEntry);

    AFortPlayerController* GetPlayerController() const;

    TArray<FFortItemEntry>& GetCurrentlyStashedItemEntries();
};
