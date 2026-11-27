#include "pch.h"
#include "Items/FortItemEntryComponent.h"
#include "Items/FortInventory.h"
#include "Items/InventoryManagementLibrary.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortItemEntryComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntryComponent"));

    return Class;
}

bool UFortItemEntryComponent::AddItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, bool bUseItemPickupAnalyticEvent)
{
    DETOUR(UFortItemEntryComponent, AddItemToInventoryOwner, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortItemEntryComponent:AddItemToInventoryOwner"))
        .Find("add [*+0x20], *")
        .Find("mov r8b, *")
        .Find("call 0x*")
        .Target()
        .Has("cmp qword ptr [rcx+0x*], 0x0")
        .Has("xor al, al; *; pop rdi; ret")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortItemEntryComponent:AddItemToInventoryOwner"))
        .Find("add [*+0x20], *")
        .Find("mov r8b, *")
        .Find("call 0x*")
        .Target()
        .Has("cmp qword ptr [rcx+0x*], 0x0")
        .Has("xor al, al; *; *; pop rdi; ret")
        .Optional()
        .AtCallsIn(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.FortItemEntryComponent:AddItemToInventoryOwner"))
            .Get()));

    if (Original(InventoryOwner, bUseItemPickupAnalyticEvent))
        return true;

    if (!HasItem())
        return false;

    return UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(InventoryOwner, GetOwnedItemEntry());
}

bool UFortItemEntryComponent::HasItem() const
{
    return const_cast<UFortItemEntryComponent*>(this)->GetOwnedItemEntry().GetItemDefinition() != nullptr;
}

FFortItemEntry& UFortItemEntryComponent::GetOwnedItemEntry()
{
    static auto OwnedItemEntry = StaticClass()->FindPropertyByName(FName(TEXT("OwnedItemEntry")));

    return *OwnedItemEntry->ContainerPtrToValuePtr<FFortItemEntry>(this);
}
