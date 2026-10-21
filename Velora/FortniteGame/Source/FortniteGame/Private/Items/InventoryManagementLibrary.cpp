#include "pch.h"
#include "Items/InventoryManagementLibrary.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Player/FortPlayerController.h"
#include "Misc/Guid.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

bool UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FFortItemEntry& ItemInstance)
{
    DETOUR(UInventoryManagementLibrary, GiveItemEntryToInventoryOwner, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.InventoryManagementLibrary:GiveItemEntryToInventoryOwner"))
        .Is("xor al, al; ret")
        .AtCallsIn(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.InventoryManagementLibrary:GiveItemEntryToInventoryOwner"))
            .Get()), Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.InventoryManagementLibrary:GiveItemEntryToInventoryOwner"))
        .Is("xor al, al; ret")
        .AtCallsIn(Anchor::Address()
            .Native(TEXT("/Script/FortniteGame.FortItemEntryComponent:AddItemToInventoryOwner"))
            .Get()));

    auto PlayerController = Cast<AFortPlayerController>(InventoryOwner.GetObject());
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;
    FFortItemEntry ItemCopy(ItemInstance);

    if (!WorldInventory || !ItemCopy.GetItemDefinition())
        return false;

    auto& Inventory = WorldInventory->GetInventory();

    for (auto& Entry : Inventory.GetReplicatedEntries())
    {
        if (Entry.GetItemGuid() == ItemCopy.GetItemGuid())
            ItemCopy.GetItemGuid() = FGuid::NewGuid();
    }

    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(ItemCopy);
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;

    if (ItemEntry->GetParentInventory().IsValid())
        ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    if (!PlayerController->CanServerTryAddToQuickBar())
        return true;

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
            PlayerController->TryAddToQuickBar(Item);
    }

    return true;
}

bool UInventoryManagementLibrary::RemoveItem(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FGuid& ItemGuid, int32 Count)
{
    DETOUR(UInventoryManagementLibrary, RemoveItem, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.InventoryManagementLibrary:RemoveItem"))
        .Is("sub rsp, 0x28; cmp qword ptr [rcx], 0x0; jnz 0x*; xor al, al; add rsp, 0x28; ret")
        .AtCallsIn(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.InventoryManagementLibrary:RemoveItem"))
            .Get()));

    if (!InventoryOwner.GetObject())
        return false;

    auto PlayerController = Cast<AFortPlayerController>(InventoryOwner.GetObject());
    auto WorldInventory = PlayerController && PlayerController->HasAuthority() ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return true;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        if (ItemEntry.GetItemGuid() == ItemGuid)
        {
            PlayerController->RemoveInventoryItem(ItemGuid, Count < 0 ? ItemEntry.GetCount() : Count, false);
            break;
        }
    }

    return true;
}
