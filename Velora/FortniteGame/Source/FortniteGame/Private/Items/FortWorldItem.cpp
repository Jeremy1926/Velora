#include "pch.h"
#include "Items/FortWorldItem.h"
#include "Items/FortInventory.h"
#include "UObject/UnrealType.h"

bool UFortWorldItem::SetLoadedAmmo(int32 NewLoadedAmmo)
{
    GetItemEntry().SetLoadedAmmo(NewLoadedAmmo);

    return true;
}

FFortItemEntry& UFortWorldItem::GetItemEntry()
{
    static auto ItemEntry = GetClass()->FindPropertyByName(FName(TEXT("ItemEntry")));

    return *ItemEntry->ContainerPtrToValuePtr<FFortItemEntry>(this);
}
