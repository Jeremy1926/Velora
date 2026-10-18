#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Set.h"
#include "Misc/Guid.h"
#include "UObject/AssetPtr.h"
#include "Items/FortItemDefinition.h"

class UFortWorldItemDefinition;

enum class EFortQuickBars : uint8
{
    Primary,
    Secondary,
    Max_None
};

struct FQuickBarSlot
{
    TArray<FGuid> Items;
    bool bEnabled;
};

struct FQuickBarSlotData
{
    TArray<EFortItemType> AcceptedItemTypes;
    bool bStaticSlot;
    TAssetPtr<UFortWorldItemDefinition> DefaultItem;
};

struct FQuickBarData
{
    TArray<FQuickBarSlotData> QuickbarSlots;
};

struct FQuickBar
{
    int32 CurrentFocusedSlot;
    int32 PreviousFocusedSlot;
    int32 SecondaryFocusedSlot;
    TArray<FQuickBarSlot> Slots;
    FQuickBarData DataDefinition;
    TSet<UFortItemDefinition*> EquippedItemDefinitions;
    TArray<int32> SharedVisibleSlotIndicesWhenUsingGamepad;
};

class AFortQuickBars : public AActor
{
public:
    static UClass* StaticClass();

    void ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot);

    FQuickBar& GetQuickBar(EFortQuickBars InQuickBar);

    void OnQuickBarChanged(EFortQuickBars InQuickBar);

    void ServerRemoveItemInternal_Implementation(const FGuid& Item, bool bFindReplacement, bool bForce);
};
