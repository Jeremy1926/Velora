#pragma once

#include "GameFramework/PlayerController.h"
#include "Misc/Guid.h"

class ABuildingSMActor;
class AFortInventory;
class AFortQuickBars;
class UFortItem;
class UFortItemDefinition;
class UFortRegisteredPlayerInfo;
enum class EFortQuickBars : uint8;
enum class EFortResourceType : uint8;

class AFortPlayerController : public APlayerController
{
public:
    static UClass* StaticClass();

    void ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int32 PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot);

    class FOnMcpProfilesInitialized;

    bool InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized);

    void AddItemToQuickBars(UFortItemDefinition* ItemDefinition, EFortQuickBars QuickBarType, int32 SlotIndex);

    void ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot);

    UFortItem* K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const;

    bool RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars);

    void RemoveAllInventoryItems();

    void RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition);

    void ServerExecuteInventoryItem_Implementation(FGuid ItemGuid);

    AFortQuickBars*& GetQuickBars();

    AFortInventory*& GetWorldInventory();

    UFortRegisteredPlayerInfo*& GetMyPlayerInfo();
};
