#pragma once

#include "Components/ActorComponent.h"
#include "UObject/ScriptInterface.h"

class IFortInventoryOwnerInterface;
struct FFortItemEntry;

class UFortItemEntryComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    bool AddItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, bool bUseItemPickupAnalyticEvent);

    bool HasItem() const;

    FFortItemEntry& GetOwnedItemEntry();
};
