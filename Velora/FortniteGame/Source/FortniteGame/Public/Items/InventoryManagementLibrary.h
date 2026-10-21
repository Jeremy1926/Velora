#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ScriptInterface.h"

class IFortInventoryOwnerInterface;
struct FFortItemEntry;

class UInventoryManagementLibrary : public UBlueprintFunctionLibrary
{
public:
    static bool GiveItemEntryToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FFortItemEntry& ItemInstance);
};
