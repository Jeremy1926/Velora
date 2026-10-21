#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ScriptInterface.h"

class IFortInventoryOwnerInterface;
struct FFortItemEntry;
struct FGuid;

class UInventoryManagementLibrary : public UBlueprintFunctionLibrary
{
public:
    static bool GiveItemEntryToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FFortItemEntry& ItemInstance);

    static bool RemoveItem(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FGuid& ItemGuid, int32 Count);
};
