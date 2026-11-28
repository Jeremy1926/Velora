#pragma once

#include "Athena/Modifiers/FortAthenaMutator.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class AFortPawn;
class UFortWorldItemDefinition;
class UScriptStruct;
struct FScalableFloat;

struct FItemsToDropOnDeath
{
    static UScriptStruct* StaticStruct();

    UFortWorldItemDefinition* GetItemToDrop() const;

    const FScalableFloat& GetNumberToDrop() const;
};

class AFortAthenaMutator_ItemDropOnDeath : public AFortAthenaMutator
{
public:
    static UClass* StaticClass();

    void SpawnItems(AFortPawn* Pawn);

    const FScalableFloat& GetPercentAmmoToDrop() const;

    const FScalableFloat& GetAmmoDespawnTime() const;

    const FScalableFloat& GetAmmoStormDespawnTime() const;

    const FScalableFloat& GetMaxAmmoTypeDrops() const;

    const FScalableFloat& GetMaxResourceTypeDrops() const;

    int32 GetNumItemsToDrop() const;

    const FItemsToDropOnDeath& GetItemsToDrop(int32 Index) const;

    TArray<FName> GetLootTierGroupsToDrop() const;
};
