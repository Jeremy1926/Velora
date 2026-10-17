#include "pch.h"
#include "Items/FortWorldItem.h"
#include "Items/FortInventory.h"
#include "UObject/UnrealType.h"

void UFortWorldItem::SetLoadedAmmo(int32 NewLoadedAmmo)
{
    GetItemEntry().SetLoadedAmmo(NewLoadedAmmo);
}

FFortItemEntry& UFortWorldItem::GetItemEntry()
{
    static auto ItemEntry = GetClass()->FindPropertyByName(FName(TEXT("ItemEntry")));

    return *ItemEntry->ContainerPtrToValuePtr<FFortItemEntry>(this);
}
