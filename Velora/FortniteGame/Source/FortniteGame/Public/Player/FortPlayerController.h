#pragma once

#include "GameFramework/PlayerController.h"
#include "Misc/Guid.h"

class AFortInventory;
class AFortQuickBars;
class UFortItem;
class UFortItemDefinition;
enum class EFortQuickBars : uint8;

class AFortPlayerController : public APlayerController
{
public:
    class FOnMcpProfilesInitialized;

    bool InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized);

    void AddItemToQuickBars(UFortItemDefinition* ItemDefinition, EFortQuickBars QuickBarType, int32 SlotIndex);

    UFortItem* K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const;

    void ServerExecuteInventoryItem_Implementation(FGuid ItemGuid);

    AFortQuickBars*& GetQuickBars();

    AFortInventory*& GetWorldInventory();
};
