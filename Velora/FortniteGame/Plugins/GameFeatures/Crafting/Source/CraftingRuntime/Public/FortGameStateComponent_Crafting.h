#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Items/FortInventory.h"
#include "UObject/NameTypes.h"

class UObject;

struct FCraftingResult
{
    FCraftingResult()
        : ResultLootTierKey(NAME_None)
    {
    }

    FName ResultLootTierKey;
    TArray<FItemAndCount> Results;
};

class UFortGameStateComponent_Crafting : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_Crafting* Get(const UObject* WorldContextObject);

    void BuildCraftingFormulaAccelerationData();

    void UpdateCraftingResultsList();

    void OnRep_CraftingResultsList();

    TArray<FCraftingResult>& GetCraftingResultsList();
};
