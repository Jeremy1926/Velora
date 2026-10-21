#include "pch.h"
#include "Athena/AI/FortAthenaAIBotInventoryItems.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

TArray<FItemAndCount> UFortAthenaAIBotInventoryItems::GetItems()
{
    static auto Items = (UArrayProperty*)GetClass()->FindPropertyByName(FName(TEXT("Items")));

    if (Items->GetInner()->GetSize() == sizeof(FItemAndCount))
        return *Items->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);

    TArray<FItemAndCount> ItemsAndCounts;

    for (auto ItemDefinition : *Items->ContainerPtrToValuePtr<TArray<UFortItemDefinition*>>(this))
        ItemsAndCounts.Add({ 1, ItemDefinition });

    return ItemsAndCounts;
}
