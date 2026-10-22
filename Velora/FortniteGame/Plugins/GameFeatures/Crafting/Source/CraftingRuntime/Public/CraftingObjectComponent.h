#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "GameplayTagContainer.h"
#include "Items/FortInventory.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortPlayerController;

enum class ECraftingObjectState : uint8
{
    Invalid,
    Idle,
    Crafting,
    Ready,
    OverCrafting,
    Resetting,
    TotalStates
};

class UCraftingObjectComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void StartCrafting(AFortPlayerController* Instigator, const FName& FormulaRow);

    void CancelCrafting();

    void ClaimCraftingResults();

    void GiveCraftingResults();

    void GiveCraftingIngredients();

    void HandleCratingObjectResetting();

    void SetCraftingObjectState(ECraftingObjectState InCraftingObjectState);

    int32 SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude);

    ECraftingObjectState GetCraftingObjectState() const;

    AFortPlayerController* GetCraftingInstigator() const;

    TArray<FItemAndCount>& GetCraftingResults();

    TArray<FFortItemEntry>& GetIngredientItems();

    const FVector& GetIngredientSpawnOffset() const;

    bool GetGiveIngredientsToCraftingObject() const;

    bool GetGiveIngredientsToInstigator() const;

    bool GetGiveToCraftingObject() const;

    bool GetGiveResultToInstigator() const;

    bool HasStateChangeTimer() const;
};
