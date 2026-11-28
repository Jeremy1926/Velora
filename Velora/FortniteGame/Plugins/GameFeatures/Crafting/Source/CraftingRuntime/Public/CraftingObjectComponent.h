#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "GameplayTagContainer.h"
#include "Items/FortInventory.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortPickup;
class AFortPlayerController;

struct FCraftingMultiKey
{
    int64 Key;
};

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

    void StartCrafting(AFortPlayerController* Instigator, const FName& FormulaRow, int32 NumberToCraft, FCraftingMultiKey Key, float OverriddenObjectStateLength);

    void PickupItemAndStartCrafting(AFortPlayerController* Instigator, AFortPickup* Pickup, const FName& FormulaRow);

    void CancelCrafting();

    void CancelCrafting(FCraftingMultiKey Key);

    void ClaimCraftingResults();

    void ClaimCraftingResults(FCraftingMultiKey Key);

    void EjectItems();

    void GiveCraftingResults();

    void GiveCraftingIngredients();

    void HandleCratingObjectResetting();

    void HandleCratingObjectResetting(FCraftingMultiKey Key);

    void SetCraftingObjectState(ECraftingObjectState InCraftingObjectState);

    void SetCraftingObjectState(FCraftingMultiKey Key, ECraftingObjectState InCraftingObjectState);

    int32 SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude);

    ECraftingObjectState GetCraftingObjectState() const;

    AFortPlayerController* GetCraftingInstigator() const;

    TArray<FItemAndCount>& GetCraftingResults();

    TArray<FFortItemEntry>& GetIngredientItems();

    TArray<int32>* GetNonConsumedIngredientItemIndices();

    FFortItemEntry& GetPendingPickupCraftingItemEntry();

    int32& GetPendingPickupHeldCount();

    FVector GetIngredientSpawnOffset() const;

    bool GetGiveIngredientsToCraftingObject() const;

    bool GetGiveIngredientsToInstigator() const;

    bool GetGiveToCraftingObject() const;

    bool GetGiveResultToInstigator() const;

    bool HasStateChangeTimer() const;
};
